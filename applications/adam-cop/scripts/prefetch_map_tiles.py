#!/usr/bin/env python3
"""
adam-cop Tile Prefetch Script
Downloads world map raster tiles for offline C2 operations into ./map_cache/
Supports all providers: cartodb_dark, openstreetmap, esri_satellite, opentopomap
"""

import os
import sys
import math
import argparse
import urllib.request
import concurrent.futures
import time

PROVIDERS = {
    "cartodb_dark": {
        "name": "CartoDB Dark Matter (Tactical)",
        "url": "https://a.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}.png",
        "folder": "cartodb_dark",
        "max_native_zoom": 19
    },
    "osm": {
        "name": "OpenStreetMap Standard",
        "url": "https://tile.openstreetmap.org/{z}/{x}/{y}.png",
        "folder": "openstreetmap",
        "max_native_zoom": 19
    },
    "esri_satellite": {
        "name": "Esri World Imagery (Satellite)",
        "url": "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}",
        "folder": "esri_satellite",
        "max_native_zoom": 18
    },
    "opentopo": {
        "name": "OpenTopoMap (Topographic)",
        "url": "https://tile.opentopomap.org/{z}/{x}/{y}.png",
        "folder": "opentopomap",
        "max_native_zoom": 17
    }
}

def lat_lon_to_tile_xy(lat: float, lon: float, z: int):
    lat = max(min(lat, 85.05112878), -85.05112878)
    n = 2 ** z
    x = int((lon + 180.0) / 360.0 * n)
    lat_rad = math.radians(lat)
    y = int((1.0 - math.log(math.tan(lat_rad) + 1.0 / math.cos(lat_rad)) / math.pi) / 2.0 * n)
    return max(0, min(x, n - 1)), max(0, min(y, n - 1))

def download_tile(provider_key: str, z: int, x: int, y: int, cache_dir: str):
    info = PROVIDERS[provider_key]
    out_dir = os.path.join(cache_dir, info["folder"], str(z), str(x))
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, f"{y}.png")

    if os.path.exists(out_path) and os.path.getsize(out_path) > 100:
        return True, "cached"

    url = info["url"].format(z=z, x=x, y=y)
    req = urllib.request.Request(url, headers={"User-Agent": "adam-cop/1.0"})

    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            data = resp.read()
            if len(data) > 0:
                with open(out_path, "wb") as f:
                    f.write(data)
                return True, "downloaded"
    except Exception as e:
        return False, str(e)

    return False, "empty response"

def main():
    parser = argparse.ArgumentParser(description="Pre-fetch world raster map tiles for adam-cop offline C2 operations")
    parser.add_argument("--min-zoom", type=int, default=0, help="Minimum zoom level (default: 0)")
    parser.add_argument("--max-zoom", type=int, default=7, help="Maximum zoom level (default: 7 for global world overview)")
    parser.add_argument("--provider", type=str, default="all", choices=list(PROVIDERS.keys()) + ["all"], help="Map provider to download")
    parser.add_argument("--bbox", type=float, nargs=4, metavar=("MIN_LAT", "MAX_LAT", "MIN_LON", "MAX_LON"), help="Optional geographic bounding box")
    parser.add_argument("--threads", type=int, default=8, help="Number of concurrent download threads (default: 8)")
    parser.add_argument("--output-dir", type=str, default="./map_cache", help="Output directory for map cache (default: ./map_cache)")

    args = parser.parse_args()

    selected_providers = list(PROVIDERS.keys()) if args.provider == "all" else [args.provider]

    print("=========================================================")
    print("        adam-cop Map Tile Pre-Fetch Utility             ")
    print("=========================================================")
    print(f"Target Cache Directory : {os.path.abspath(args.output_dir)}")
    print(f"Providers              : {', '.join(selected_providers)}")
    print(f"Zoom Level Range       : {args.min_zoom} to {args.max_zoom}")
    if args.bbox:
        print(f"Bounding Box           : Lat [{args.bbox[0]}, {args.bbox[1]}], Lon [{args.bbox[2]}, {args.bbox[3]}]")
    else:
        print(f"Coverage               : Full Global World Map")
    print("=========================================================")

    tasks = []
    for prov in selected_providers:
        max_native = PROVIDERS[prov]["max_native_zoom"]
        effective_max = min(args.max_zoom, max_native)

        for z in range(args.min_zoom, effective_max + 1):
            if args.bbox:
                min_lat, max_lat, min_lon, max_lon = args.bbox
                x_min, y_max = lat_lon_to_tile_xy(min_lat, min_lon, z)
                x_max, y_min = lat_lon_to_tile_xy(max_lat, max_lon, z)
            else:
                n = 2 ** z
                x_min, x_max = 0, n - 1
                y_min, y_max = 0, n - 1

            for x in range(x_min, x_max + 1):
                for y in range(y_min, y_max + 1):
                    tasks.append((prov, z, x, y))

    total_tasks = len(tasks)
    print(f"Total tiles to verify/fetch: {total_tasks:,}")
    if total_tasks == 0:
        print("Nothing to do.")
        return

    completed = 0
    downloaded = 0
    cached = 0
    failed = 0

    start_time = time.time()

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.threads) as executor:
        futures = {
            executor.submit(download_tile, prov, z, x, y, args.output_dir): (prov, z, x, y)
            for prov, z, x, y in tasks
        }

        for future in concurrent.futures.as_completed(futures):
            completed += 1
            success, status = future.result()

            if success:
                if status == "cached":
                    cached += 1
                else:
                    downloaded += 1
            else:
                failed += 1

            if completed % 100 == 0 or completed == total_tasks:
                elapsed = time.time() - start_time
                speed = completed / elapsed if elapsed > 0 else 0
                pct = (completed / total_tasks) * 100.0
                print(f"Progress: [{completed:,}/{total_tasks:,}] ({pct:.1f}%) | Cached: {cached:,} | Downloaded: {downloaded:,} | Failed: {failed} | Speed: {speed:.1f} tiles/sec", end="\r")

    print("\n=========================================================")
    print("Pre-fetch Completed!")
    print(f"Total Tiles Verified : {completed:,}")
    print(f"Already Cached       : {cached:,}")
    print(f"Newly Downloaded     : {downloaded:,}")
    print(f"Failed               : {failed:,}")
    print(f"Total Time           : {time.time() - start_time:.2f} seconds")
    print("=========================================================")

if __name__ == "__main__":
    main()

# Code Style
- Always ensure that opening braces { are placed on a new line (Allman style).

# Syntax/Logic
- Always use adam::string_hashed adam::string_hashed_ct or adam::string_hash when using strings. Prefer adam::string_hashed_ct when possible. And when you actually need to pass a string, use const reference. For comparison only, use string_hash
- Always use guard clauses when possible.


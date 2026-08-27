# Code Style
- Always ensure that opening braces { are placed on a new line (Allman style). But only for multiline blocks, single line code does not need to follow this rule.
- Ensure getter/setter are single line and use the inline keyword
- Always group const functions together first in the class, the all non const functions
- Please align all function names, variable names and inline function definition by indenting

# Syntax/Logic
- Always use adam::string_hashed adam::string_hashed_ct or adam::string_hash when using strings. Prefer adam::string_hashed_ct when possible. And when you actually need to pass a string, use const reference. For comparison only, use string_hash
- Always use guard clauses when possible.


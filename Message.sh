#!/bin/sh
# This script use find and xgettext command to generate the translation template file for openfetion
find . -type f -iname "*.c" | xgettext --files-from=- --output=po/hybrid.pot --from-code=utf-8 --sort-by-file --language=C --keyword=_ --keyword=gettext --keyword=i18n --keyword=N_ --add-comments --add-location --package-name=Hybrid --msgid-bugs-address=https://github.com/levin108/hybrid/issues

set e = "`echo x | /bin/tr x \\033`"
alias ko 'echo "${e}(J"; echo "*** FORCED KANJI-OUT ***"'

#/bin/bash

git add -A

echo "Please provide a commit message."

read message

git commit -m "$message"
git push origin master


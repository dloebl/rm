# rm
Command-line utility that can delete directory entries.
Written in C, requires an at least partly POSIX conforming operating system underneath.

The goal of this tool is, to fulfill the corresponding POSIX standard as good as possible, without adding more features.

## Sytax
* rm [-iRr] file...
* rm -f [-iRr] [file...]

## Options
* -f 
  * Forced mode, errors will be ignored. Will not prompt for any confirmation from the user.
* -i
  * Interactive mode. Will prompt for confirmation from the user on every single item to be deleted.
* -r *or* -R
  * Recursive mode. When set, it is possible to delete whole directories.

## Notes
Does NOT include any security features (e.g. preserve-root).

*For more details about the usage, please refer to:* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/rm.html

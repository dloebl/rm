# rm
Simple implementation of the rm utility in C for unix-like systems. 
Can delete directory entries.
Should conform to the POSIX standard.

## Sytax
* rm [-iRr] file...
* rm -f [-iRr] [file...]

## Options
* -f 
  * Forced mode, errors will be ignored. Will not prompt for any confimation from the user.
* -i
  * Interactive mode. Will prompt for confirmation from the user on every single item to be deleted.
* -r *or* -R
  * Recursive mode. If set, it is possible to delete whole directories.

## Notes
Does NOT include any security features (e.g. preserve-root).

*For more details about the usage, please refer to:* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/rm.html

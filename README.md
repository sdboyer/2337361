Drush update modules
=========

Allows 'update modules' to be added to a Drupal installation, instead of or in addition to using hook_update_N(.

Each module should add:

update type=before
or
update type=after

to the .info file.

When drush updb runs, all modules that are found will be installed, either
before or after the actual hook_update_N() runs.

This is primarily useful for sites under heavy development to avoid
hook_update_N() conflicts.

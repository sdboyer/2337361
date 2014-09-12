<?php

/**
 * Implements drush_hook_pre_COMMAND().
 */
function drush_drush_update_modules_pre_updatedb() {
  drush_bootstrap_max();
  drush_update_modules('before');
}

/**
 * Implements drush_hook_post_COMMAND().
 */
function drush_drush_update_modules_post_updatedb() {
  drush_bootstrap_max();
  drush_update_modules('after');
}

/**
 * Enable update modules.
 *
 * @param $type
 *   The type as specified in update type=before|after
 */
function drush_update_modules($type) {
  // Get the data built during the validate phase
  $modules = array();

  foreach (system_rebuild_module_data() as $module => $data) {
    if (!module_exists($module) && !empty($data->info['update type']) && $data->info['update type'] == $type) {
      $modules[] = $module;
    }
  }

  if (empty($modules)) {
    return drush_log(dt('There were no extensions that could be enabled.'), 'ok');
  }
  else {
    drush_print(dt('The following update modules will be enabled: !modules', array('!modules' => implode(', ', $modules))));
    if(!drush_confirm(dt('Do you really want to continue?'))) {
      return drush_user_abort();
    }
  }

  // Enable modules and pass dependency validation in form submit.
  if (!empty($modules)) {
    drush_include_engine('drupal', 'environment');
    drush_module_enable($modules);
  }
}

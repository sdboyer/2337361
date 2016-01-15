<?php

function drush_update_modules_drush_command() {
  return array(
    'generate-update-module' => array(
      'description' => 'Generates an empty update module, ready for customization.',
      'arguments' => array(
        'name' => 'The name to use for the generated module.',
      ),
      'options' => array(
        'update-type' => array(
          'description' => 'Whether this update runs "before" or "after" normal updates. Defaults to "after".',
          'example-value' => 'after',
        ),
        'parent-dir' => array(
          'description' => 'The parent directory in which to generate the module. Specify the full path, relative to docroot. (Defaults to sites/all/modules/updates)',
          'example-value' => 'sites/all/modules/updates',
        ),
        'title' => array(
          'description' => 'Human-readable title to use for the module. Defaults to same as module machine name.',
        ),
        //'desc' => array(
          //'description' => 'Description for the module .info file. Defaults to empty string.',
        //),
      ),
      'aliases' => array('gum'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_SITE,
      'drush dependencies' => array('pm'),
    )
  );
}

/**
 * Generate a template update module.
 *
 * @param $name
 *   The name of the module to generate.
 */
function drush_drush_update_modules_generate_update_module($name) {
  if ($name == '') {
    return drush_set_error('NONAME', 'No module name provided');
  }

  if (count(func_get_args()) > 1) {
    return drush_set_error('MORENAMES', 'Can only operate on one name at a time');
  }

  if (!preg_match('/^[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*$/', $name)) {
    return drush_set_error('BADNAME', 'Invalid characters for machine name');
  }

  $info = array(
    //'desc' => drush_get_option('desc', $name),
    'title' => drush_get_option('title', $name),
    'type' => drush_get_option('update-type', 'after'),
    'parent-dir' => drush_get_option('parent-dir', ''),
    'core' => drush_drupal_major_version() . '.x',
  );

  if (empty($info['parent-dir'])) {
    // Fuck it, if drush can't make it clear how to load these, then hack it in
    require_once DRUSH_BASE_PATH . '/commands/pm/download.pm.inc';
    $base = _pm_download_destination('module');
    // strip contrib dir, use updates base instead. or just append, if contrib
    // isn't there
    $info['parent-dir'] = rtrim($base, '/contrib') . '/updates';
  }

  if (!file_exists($info['parent-dir'])) {
    if (!drush_confirm(dt('Updates directory !path does not exist. Do you want to create it?', array('!path' => $info['parent-dir'])))) {
      return drush_user_abort();
    }
    drush_mkdir($info['parent-dir'], TRUE);
  }

  $tdir = $info['parent-dir'] . '/' . $name;
  if (file_exists($tdir)) {
    drush_log(dt('Module directory already exists at !dir', array('!dir' => $tdir)));
    return drush_set_error('MODULEEXISTS', 'Module already exists');
  }

  drush_mkdir($tdir);
  file_put_contents($tdir . "/$name.info", <<<EOT
name = {$info['title']}
update type = '{$info['type']}'
core = {$info['core']}

EOT
);

  file_put_contents($tdir . "/$name.module", <<<'EOT'
<?php

/**
 * Intentionally left blank.
 */

EOT
);

  file_put_contents($tdir . "/$name.install", <<<EOT
<?php

/**
 * Implements hook_install().
 */
function {$name}_install() {

}

EOT
);
  drush_log(dt('Update module created with .info, .module, and .install file at !dir', array('!dir' => $tdir)));
}

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

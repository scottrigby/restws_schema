<?php
/**
 * @file
 * pubapi_example.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function pubapi_example_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'access resource blog'.
  $permissions['access resource blog'] = array(
    'name' => 'access resource blog',
    'roles' => array(
      'administrator' => 'administrator',
      'anonymous user' => 'anonymous user',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'restws',
  );

  // Exported permission: 'access resource episode'.
  $permissions['access resource episode'] = array(
    'name' => 'access resource episode',
    'roles' => array(
      'administrator' => 'administrator',
      'anonymous user' => 'anonymous user',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'restws',
  );

  // Exported permission: 'access resource season'.
  $permissions['access resource season'] = array(
    'name' => 'access resource season',
    'roles' => array(
      'administrator' => 'administrator',
      'anonymous user' => 'anonymous user',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'restws',
  );

  // Exported permission: 'access resource show'.
  $permissions['access resource show'] = array(
    'name' => 'access resource show',
    'roles' => array(
      'administrator' => 'administrator',
      'anonymous user' => 'anonymous user',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'restws',
  );

  return $permissions;
}

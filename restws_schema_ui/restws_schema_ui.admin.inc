<?php

/**
 * @file
 * RestWS Schema UI forms.
 */

/**
 * RestWS Schema settings form constructor.
 *
 * Maps RestWS Schema keys to entity types, bundles & properties.
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function restws_schema_ui_settings($form, &$form_state) {
  $schema = restws_schema_get();
  $map = restws_schema_map_get();

  foreach ($schema as $resource => $info) {
    $form[$resource] = array(
      '#type' => 'fieldset',
      '#title' => $info['label'],
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      // Add stable wrapper ID.
      '#prefix' => '<div id="' . $resource . '-bundle-wrapper">',
      '#suffix' => '</div>',
    );
    restws_schema_ui_entity_type_settings($form[$resource], $form_state, $resource, $map);
  }

  return system_settings_form($form);
}

/**
 * Form builder: Maps RestWS Schema keys to entity types.
 *
 * @param array $form
 *   By reference.
 * @param array $form_state
 *   By reference.
 * @param string $resource
 *   The restws resource key, as defined in hook_restws_resource_info().
 * @param array $map
 *   The restws_schema map array, as returned by restws_schema_map_get().
 */
function restws_schema_ui_entity_type_settings(&$form, &$form_state, $resource, $map) {
  $entity_types = array();
  foreach (entity_get_info() as $entity_type => $entity_info) {
    $entity_types[$entity_type] = check_plain($entity_info['label']);
  }

  // We want to reflect stored values, but prioritize current ajax interaction.
  $key = "restws_schema_ui_entity_type_{$resource}";
  $entity_type_selected = isset($form_state['input'][$key]) ? $form_state['input'][$key] : variable_get($key, '');
  $form[$key] = array(
    '#title' => t('Entity type'),
    '#options' => $entity_types,
    '#default_value' => $entity_type_selected,
  ) + restws_schema_ui_element_common() + restws_schema_ui_ajax_common($resource);

  if ($entity_type_selected) {
    restws_schema_ui_bundle_settings($form, $form_state, $resource, $map, $entity_type_selected);
  }
}

/**
 * Form builder: Maps RestWS Schema keys to bundles.
 *
 * @param array $form
 *   By reference.
 * @param array $form_state
 *   By reference.
 * @param string $resource
 *   The restws resource key, as defined in hook_restws_resource_info().
 * @param array $map
 *   The restws_schema map array, as returned by restws_schema_map_get().
 * @param string $entity_type
 *   The selected Entity type.
 */
function restws_schema_ui_bundle_settings(&$form, &$form_state, $resource, $map, $entity_type) {
  $entity_type_info = entity_get_info();
  $bundles = $entity_type_info[$entity_type]['bundles'];
  $bundle_options = array();
  foreach ($bundles as $bundle => $bundle_info) {
    $bundle_options[$bundle] = check_plain($bundle_info['label']);
  }
  $key = "restws_schema_ui_bundle_{$resource}";
  $bundle_selected = isset($form_state['input'][$key]) ? $form_state['input'][$key] : variable_get($key, '');
  $form[$key] = array(
    '#title' => t('Bundle'),
    '#options' => $bundle_options,
    '#default_value' => $bundle_selected,
  ) + restws_schema_ui_element_common() + restws_schema_ui_ajax_common($resource);

  if ($bundle_selected) {
    restws_schema_ui_property_settings($form, $form_state, $resource, $map, $entity_type, $bundle_selected);
  }
}

/**
 * Form builder: Maps RestWS Schema keys to properties.
 *
 * @param array $form
 *   By reference.
 * @param array $form_state
 *   By reference.
 * @param string $resource
 *   The restws resource key, as defined in hook_restws_resource_info().
 * @param array $map
 *   The restws_schema map array, as returned by restws_schema_map_get().
 * @param string $entity_type
 *   The selected Entity type.
 * @param string $bundle
 *   The selected Bundle.
 */
function restws_schema_ui_property_settings(&$form, &$form_state, $resource, $map, $entity_type, $bundle) {
  $schema = restws_schema_get();;

  // Get properties.
  // Limit only to properties on the selected bundle.
  // @see entity_get_all_property_info()
  $entity_properties = entity_get_property_info($entity_type);
  $entity_properties += array('properties' => array(), 'bundles' => array());
  // Add the selected bundle's properties.
  if (isset($entity_properties['bundles'][$bundle])) {
    $bundle_info = $entity_properties['bundles'][$bundle];
    $bundle_info += array('properties' => array());
    $entity_properties['properties'] += $bundle_info['properties'];
  }

  $property_options = array();
  foreach ($entity_properties['properties'] as $property_name => $property_info) {
    $property_info_label = array();
    foreach(array('label', 'description') as $property_info_key) {
      if (isset($property_info[$property_info_key])) {
        $property_info_label[] = $property_info[$property_info_key];
      }
    }
    $property_options[$property_name] = implode(': ', $property_info_label);
  }
  // Get restws_schema resource properties.
  foreach ($schema[$resource]['properties'] as $property_name => $property_info) {
    $key = "restws_schema_ui_{$resource}_property_{$property_name}";
    $property_selected = isset($form_state['input'][$key]) ? $form_state['input'][$key] : variable_get($key, '');
    $form[$key] = array(
      '#title' => $property_info['label'],
      '#options' => $property_options,
      '#default_value' => $property_selected,
    ) + restws_schema_ui_element_common();
  }
}

/**
 * Defines common form element items.
 *
 * @return array
 *   An array of common form element items.
 */
function restws_schema_ui_element_common() {
  return array(
    '#type' => 'select',
    '#empty_option' => t('- Please select -'),
    '#required' => TRUE,
  );
}

/**
 * Defines common form ajax items.
 *
 * @param string $resource
 *   The restws resource key, as defined in hook_restws_resource_info(). This
 *   tells ajax which resource form container to reload.
 * @return array
 *   An array of common form ajax items.
 */
function restws_schema_ui_ajax_common($resource) {
  return array(
    '#ajax' => array(
      'callback' => 'restws_schema_ui_bundle_settings_ajax_callback',
      'wrapper' => "{$resource}-bundle-wrapper",
      'effect' => 'fade',
    ),
  );
}

/**
 * Ajax callback for restws_schema_ui_settings().
 *
 * @see ajax_form_callback()
 */
function restws_schema_ui_bundle_settings_ajax_callback($form, $form_state) {
  $form_state['rebuild'] = TRUE;

  // Find the parent elements.
  $parents = $form_state['triggering_element']['#array_parents'];
  // Load one parent up.
  array_pop($parents);

  return drupal_array_get_nested_value($form, $parents);
}

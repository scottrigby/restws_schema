<?php

/**
 * @file
 * Publisher API UI forms.
 */

/**
 * Publisher API settings form constructor.
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function pubapi_ui_settings($form, &$form_state) {
  $structure = pubapi_get_structure();

  foreach ($structure as $resource => $info) {
    $form[$resource] = array(
      '#type' => 'fieldset',
      '#title' => $info['label'],
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      // Add stable wrapper ID.
      '#prefix' => '<div id="' . $resource . '-bundle-wrapper">',
      '#suffix' => '</div>',
    );
    pubapi_ui_bundle_settings($form[$resource], $form_state, $resource, $info);
  }

  return system_settings_form($form);
}

/**
 * Form builder: Maps Publisher API keys to entity types, bundles & properties.
 *
 * @param array $form
 *   By reference.
 * @param array $form_state
 *   By reference.
 * @param string $resource
 *   The restws resource key, as defined in hook_restws_resource_info().
 * @param array $info
 *   The resource info array, as returned by pubapi_get_structure().
 */
function pubapi_ui_bundle_settings(&$form, &$form_state, $resource, $info) {
  $common = pubapi_ui_element_common($resource);

  $entity_type_info = entity_get_info();
  $entity_types = array();
  foreach ($entity_type_info as $entity_type => $entity_info) {
    $entity_types[$entity_type] = check_plain($entity_info['label']);
  }

  // We want to reflect stored values, but prioritize current ajax interaction.
  $key = "pubapi_ui_entity_type_{$resource}";
  $entity_type_selected = isset($form_state['input'][$key]) ? $form_state['input'][$key] : variable_get($key, '');
  $form[$key] = array(
    '#title' => t('Entity type'),
    '#options' => $entity_types,
    '#default_value' => $entity_type_selected,
  ) + $common;

  if ($entity_type_selected) {
    $bundles = $entity_type_info[$entity_type_selected]['bundles'];
    $bundle_options = array();
    foreach ($bundles as $bundle => $bundle_info) {
      $bundle_options[$bundle] = check_plain($bundle_info['label']);
    }
    $key = "pubapi_ui_bundle_{$resource}";
    $bundle_selected = isset($form_state['input'][$key]) ? $form_state['input'][$key] : variable_get($key, '');
    $form[$key] = array(
        '#title' => t('Bundle'),
        '#options' => $bundle_options,
        '#default_value' => $bundle_selected,
      ) + $common;

    // Get properties.
    // @todo Consider limiting to only properties on the selected bundle.
    $entity_properties = entity_get_all_property_info($entity_type_selected);
    $property_options = array();
    foreach ($entity_properties as $property_name => $property_info) {
      $property_info_label = array();
      foreach(array('label', 'description') as $property_info_key) {
        if (isset($property_info[$property_info_key])) {
          $property_info_label[] = $property_info[$property_info_key];
        }
      }
      $property_options[$property_name] = implode(': ', $property_info_label);
    }
    // Get API resource properties.
    foreach ($info['properties'] as $property_name => $property_info) {
      $key = "pubapi_ui_{$resource}_property_{$property_name}";
      $property_selected = isset($form_state['input'][$key]) ? $form_state['input'][$key] : variable_get($key, '');
      $form[$key] = array(
          '#title' => $property_info['label'],
          '#options' => $property_options,
          '#default_value' => $property_selected,
        ) + pubapi_ui_element_common($resource, FALSE);
    }
  }
}

/**
 * Defines common form element items.
 *
 * @param string $resource
 *   The restws resource key, as defined in hook_restws_resource_info().
 * @param bool $ajax
 *   Whether or not to attach the Ajax callback to this element. Defaults to
 *   TRUE.
 * @return array
 *   An array of common form element items.
 */
function pubapi_ui_element_common($resource, $ajax = TRUE) {
  $common = array(
    '#type' => 'select',
    '#empty_option' => t('- Please select -'),
    '#required' => TRUE,
  );
  if ($ajax) {
    $common += array(
      '#ajax' => array(
        'callback' => 'pubapi_ui_bundle_settings_ajax_callback',
        'wrapper' => "{$resource}-bundle-wrapper",
        'effect' => 'fade',
      ),
    );
  }
  return $common;
}

/**
 * Ajax callback for pubapi_ui_settings().
 *
 * @see ajax_form_callback()
 */
function pubapi_ui_bundle_settings_ajax_callback($form, $form_state) {
  $form_state['rebuild'] = TRUE;

  // Find the parent elements.
  $parents = $form_state['triggering_element']['#array_parents'];
  // Load one parent up.
  array_pop($parents);

  return drupal_array_get_nested_value($form, $parents);
}

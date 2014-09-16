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

  $unmapped_properties = restws_schema_ui_get_mapping_status();
  if (!empty($unmapped_properties)) {
    $form['mapping_status'] = array(
      '#type' => 'markup',
      '#markup' => 'The following resources are not mapped.' .
        theme_item_list(array(
          'items' => $unmapped_properties,
          'type' => 'ul',
          'title' => '',
          'attributes' => array(),
        ))
    );
  }

  foreach ($schema as $resource => $info) {
    $form[$resource] = array(
      '#type' => 'fieldset',
      '#title' => $info['label'],
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      // Add stable wrapper ID.
      '#prefix' => '<div id="' . $resource . '-wrapper">',
      '#suffix' => '</div>',
    );
    restws_schema_ui_entity_type_settings($form[$resource], $form_state, $resource, $map);
  }

  // We don't use system_settings_form() because we want a single variable,
  // rather than one variable per schema map key.
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration'));
  if (!empty($_POST) && form_get_errors()) {
    drupal_set_message(t('The settings have not been saved because of the errors.'), 'error');
  }
  $form['#theme'] = 'system_settings_form';

  return $form;
}

/**
 * Form submission handler for restws_schema_ui_settings().
 */
function restws_schema_ui_settings_submit($form, &$form_state) {
  form_state_values_clean($form_state);

  $values = array();
  foreach ($form_state['values'] as $key => $value) {
    if (is_array($value) && isset($form_state['values']['array_filter'])) {
      $value = array_keys(array_filter($value));
    }
    $values[$key] = $value;
  }
  variable_set('restws_schema_map', $values);

  drupal_set_message(t('The configuration options have been saved.'));
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
  $options = array();
  foreach (entity_get_info() as $name => $info) {
    $options[$name] = check_plain($info['label']);
  }
  asort($options, SORT_NATURAL | SORT_FLAG_CASE);

  $entity = restws_schema_ui_get_selected($form_state, $resource, $map, 'entity', $element);
  $form[$element] = array(
      '#title' => t('Entity type'),
      '#options' => $options,
      '#default_value' => $entity,
    ) + restws_schema_ui_element_common() + restws_schema_ui_ajax_common($resource);

  if ($entity) {
    restws_schema_ui_bundle_settings($form, $form_state, $resource, $map, $entity);
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
 * @param string $entity
 *   The selected Entity type.
 */
function restws_schema_ui_bundle_settings(&$form, &$form_state, $resource, $map, $entity) {
  $entity_info = entity_get_info();
  $options = array();
  foreach ($entity_info[$entity]['bundles'] as $name => $info) {
    $options[$name] = check_plain($info['label']);
  }
  asort($options, SORT_NATURAL | SORT_FLAG_CASE);

  $bundle = restws_schema_ui_get_selected($form_state, $resource, $map, 'bundle', $element);
  $form[$element] = array(
      '#title' => t('Bundle'),
      '#options' => $options,
      '#default_value' => $bundle,
    ) + restws_schema_ui_element_common() + restws_schema_ui_ajax_common($resource);

  if ($bundle) {
    restws_schema_ui_property_settings($form, $form_state, $resource, $map, $entity, $bundle);
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
 * @param string $entity
 *   The selected Entity type.
 * @param string $bundle
 *   The selected Bundle.
 */
function restws_schema_ui_property_settings(&$form, &$form_state, $resource, $map, $entity, $bundle) {
  $schema = restws_schema_get();

  // Get properties of the entity type.
  $info = entity_get_property_info($entity);
  $properties = isset($info['properties']) ? $info['properties'] : array();
  // Get properties of the selected bundle.
  if (isset($info['bundles'][$bundle]['properties'])) {
    $properties += $info['bundles'][$bundle]['properties'];
  }

  $options = array();
  foreach ($properties as $name => $info) {
    $label = array();
    foreach(array('label', 'description') as $key) {
      if (isset($info[$key])) {
        $label[] = $info[$key];
      }
    }
    $options[$name] = implode(': ', $label);
  }
  asort($options, SORT_NATURAL | SORT_FLAG_CASE);

  // Get restws_schema resource properties.
  foreach ($schema[$resource]['properties'] as $name => $info) {
    $property = restws_schema_ui_get_selected($form_state, $resource, $map, $name, $element);
    $form[$element] = array(
        '#title' => $info['label'],
        '#options' => $options,
        '#default_value' => $property,
      ) + restws_schema_ui_element_common();
  }
}

/**
 * Gets the selected map value of an element.
 *
 * @param array $form_state
 *   The form state. This detects ajax interaction.
 * @param string $resource
 *   The restws resource key, as defined in hook_restws_resource_info().
 * @param array $map
 *   The restws_schema map array, as returned by restws_schema_map_get().
 * @param string $key
 *   The restws_schema map key.
 * @param string $element
 *   By reference. An element key unique to this resource is set.
 *
 * @return string
 *   The selected map value. We want to prioritize current ajax interaction, but
 *   fall back to stored values if any.
 */
function restws_schema_ui_get_selected($form_state, $resource, $map, $key, &$element) {
  $element = $resource . ':' . $key;
  $stored = isset($map[$element]) ? $map[$element] : '';
  $selected = isset($form_state['input'][$element]) ? $form_state['input'][$element] : $stored;

  return $selected;
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
  );
}

/**
 * Defines common form ajax items.
 *
 * @param string $resource
 *   The restws resource key, as defined in hook_restws_resource_info(). This
 *   tells ajax which resource form container to reload.
 *
 * @return array
 *   An array of common form ajax items.
 */
function restws_schema_ui_ajax_common($resource) {
  return array(
    '#ajax' => array(
      'callback' => 'restws_schema_ui_settings_ajax_callback',
      'wrapper' => "{$resource}-wrapper",
      'effect' => 'fade',
    ),
  );
}

/**
 * Ajax callback for restws_schema_ui_settings().
 *
 * @see ajax_form_callback()
 */
function restws_schema_ui_settings_ajax_callback($form, $form_state) {
  $form_state['rebuild'] = TRUE;

  // Find the parent elements.
  $parents = $form_state['triggering_element']['#array_parents'];
  // Load one parent up.
  array_pop($parents);

  return drupal_array_get_nested_value($form, $parents);
}

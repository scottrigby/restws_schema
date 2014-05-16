<?php

class PubApiResourceController
  extends RestWSEntityResourceController
  implements RestWSQueryResourceControllerInterface {

  protected $resource, $apiMap, $propertyInfo, $bundleName;

  public function __construct($name, $info) {
    $this->apiMap = pubapi_get_map();
    $this->propertyInfo = $info['properties'];
    $this->resource = $name;

    $this->entityType = $this->apiMap[$name]['entity'];
    $this->bundleName = $this->apiMap[$name]['bundle'];
    $this->entityInfo = entity_get_info($this->apiMap[$name]['entity']);
  }

  /**
   * @see RestWSEntityResourceController::propertyInfo()
   */
  public function propertyInfo() {
    return $this->propertyInfo;
  }

  /**
   * @see RestWSEntityResourceController::propertyInfo()
   */
  public function originalPropertyInfo() {
    return entity_get_all_property_info($this->entityType);
  }

  /**
   * @see RestWSResourceControllerInterface::wrapper()
   */
  public function wrapper($id) {
    $object = $this->objectLoad($id);

    return entity_metadata_wrapper($this->resource(), $object, array('property info' => $this->propertyInfo));
  }

  protected function originalWrapper($id) {
    return parent::wrapper($id);
  }

  /**
   * @see RestWSResourceControllerInterface::read()
   */
  public function read($id) {
    return $this->objectLoad($id);
  }

  /**
   * @see RestWSResourceControllerInterface::access()
   */
  public function access($op, $id) {
    // Check entity access on the original entity, not our API object (which
    // will fail).
    return entity_access($op, $this->entityType, isset($id) ? $this->originalWrapper($id)->value() : NULL);
  }

  /**
   * @see RestWSResourceControllerInterface::resource()
   *
   * @return string
   */
  public function resource() {
    return $this->resource;
  }

  /**
   * Loads a Publisher API object.
   *
   * @param int $id
   *   The original entity ID.
   *
   * @return stdClass
   *   An object matching the requested Publisher API object structure.
   */
  protected function objectLoad($id) {
    global $base_url;

    $object = new stdClass();

    // Get original entity.
    $original_wrapper = $this->originalWrapper($id);

    // If the wrapped entity is not of the correct bundle, bail now.
    if ($original_wrapper->getBundle() !== $this->bundleName) {
      return $object;
    }

    $original_properties = $original_wrapper->getPropertyInfo();

    // Add to our object according to our defined API property info.
    if ($map = $this->apiMap[$this->resource()]) {
      foreach (array_keys($this->propertyInfo) as $property) {
        if (array_key_exists($map[$property], $original_properties)) {
          $value = $original_wrapper->{$map[$property]}->value(array('sanitize' => TRUE));

          // For now make a quick check for references, and get just the ID, or
          // an array of IDs (depending on cardinality).
          // @todo abstract this with a getter callback wrapper or something for
          //   entityreference.
          // @see RestWSBaseFormat::getResourceReferenceValue
          if (($field = field_info_field($map[$property])) && $field['type'] == 'entityreference' && $type = $field['settings']['target_type']) {
            // Return an array of values, regardless of field cardinality.
            $value = !is_array($value) ? array($value) : $value;
            $values = array();
            foreach ($value as $item) {
              list($id,,$target_bundle) = entity_extract_ids($type, $item);
              $target_resource = NULL;
              // Get target resource from map.
              foreach ($this->apiMap as $r => $i) {
                if ($i['entity'] == $type && $i['bundle'] == $target_bundle) {
                  $target_resource = $r;
                  break;
                }
              }
              $values[] = (object) array(
                'uri' => $base_url . base_path() . $this->resource() . '/' . $id . '.json',
                'id' => $id,
                'resource' => $target_resource,
              );
            }
            $value = $values;
          }
          // Check for other things, like the body field. Example for
          // body: $wrapper->body->value->value(array('decode' => TRUE));
          // @todo This is way to specific. Abstract the hell out of this.
          //   Maybe get the column, like `$col = key($field['columns'])`.
          elseif (is_array($value)) {
            try {
              $value = $original_wrapper->{$map[$property]}->value->value(array('decode' => TRUE));
            }
            catch(EntityMetadataWrapperException $e) {}
          }
          $object->{$property} = $value;
        }
      }
    }

    return $object;
  }

  /**
   * @see RestWSEntityResourceController::query()
   *
   * We must override this because $this->resource() assumes entity type. Also
   * we must filter by bundle, according to our map.
   */
  public function query($filters = array(), $meta_controls = array()) {
    $limit = variable_get('restws_query_max_limit', 100);
    $offset = 0;

    $query = new EntityFieldQuery();
//    $query->entityCondition('entity_type', $this->resource());
    $query->entityCondition('entity_type', $this->entityType);

    // Also filter by bundle, according to our map.
    // If the entity type provides no bundle key: assume a single bundle, named
    // after the entity type.
    $bundle_key = isset($this->entityInfo['entity keys']['bundle']) ? $this->entityInfo['entity keys']['bundle'] : $this->entityType;
    // @todo Resolve missing field/schema keys in our property info.
    //   propertyQueryOperation::RestWSEntityResourceController assumes one or
    //   the other.
    $this->propertyQueryOperation($query, 'Condition', $bundle_key, $this->bundleName);

    // @todo Map filters.
    foreach ($filters as $filter => $value) {
      $entity_filter = $this->apiMap[$this->resource()][$filter];
      $this->propertyQueryOperation($query, 'Condition', $entity_filter, $value);
    }

    $rest_controls = restws_meta_controls();
    foreach ($meta_controls as $control_name => $value) {
      switch ($control_name) {
        case $rest_controls['sort']:
          if (isset($meta_controls[$rest_controls['direction']]) && strtolower($meta_controls[$rest_controls['direction']]) == 'desc') {
            $direction = 'DESC';
          }
          else {
            $direction = 'ASC';
          }
          $this->propertyQueryOperation($query, 'OrderBy', $value, $direction);
          break;

        case $rest_controls['limit']:
          $limit = $this->limit($value);
          break;

        case $rest_controls['page']:
          $offset = $value > 0 ? $value : $offset;
          break;
      }
    }

    // Calculate the offset.
    $offset *= $limit;
    $query->range($offset, $limit);

    $this->nodeAccess($query);

    // Catch any errors, like wrong keywords or properties.
    try {
      $query_result = $query->execute();
    }
    catch (PDOException $exception) {
      throw new RestWSException('Query failed.', 400);
    }
//    $query_result = isset($query_result[$this->resource()]) ? $query_result[$this->resource()] : array();
    $query_result = isset($query_result[$this->entityType]) ? $query_result[$this->entityType] : array();

    $result = array_keys($query_result);

    return $result;
  }

  /**
   * @see RestWSEntityResourceController::count()
   *
   * We must override this because $this->resource() assumes entity type.
   */
  public function count($filters = array()) {
    $query = new EntityFieldQuery();
//    $query->entityCondition('entity_type', $this->resource());
    $query->entityCondition('entity_type', $this->entityType);

    // @todo Map filters.
    foreach ($filters as $filter => $value) {
      $entity_filter = $this->apiMap[$this->resource()][$filter];
      $this->propertyQueryOperation($query, 'Condition', $entity_filter, $value);
    }
    $query->count();
    $this->nodeAccess($query);

    return $query->execute();
  }

  /**
   * @see RestWSEntityResourceController::propertyQueryOperation()
   *
   * We must override this in order to map the fields before querrying.
   *
   * @todo Consider submitting a patch to restws for a new method
   *   queryPropertymap(). If the parent method gave another layer between
   *   propertyInfo() (which is used elsewhere, so must be set to our custom
   *   object properties), and the query implementation, we could intercept it
   *   and do mapping there. Until then, we need to override this entire method.
   */
  protected function propertyQueryOperation(EntityFieldQuery $query, $operation, $property, $value) {
//    $properties = $this->propertyInfo();
    // Get the original entity property info.
    $properties = $this->originalPropertyInfo();
    // Reset the $property key to the original entity's mapped property.
//    $resource = $this->resource();
//    $property = $this->apiMap[$this->resource()][$property];

    // If field is not set, then the filter is a property and we can extract
    // the schema field from the property array.
    if (empty($properties[$property]['field'])) {
      $column = $properties[$property]['schema field'];
      $operation = 'property' . $operation;
      $query->$operation($column, $value);
    }
    else {
      // For fields we need the field info to get the right column for the
      // query.
      $field_info = field_info_field($property);
      $operation = 'field' . $operation;
      if (is_array($value)) {
        // Specific column filters are given, so add a query condition for each
        // one of them.
        foreach ($value as $column => $val) {
          $query->$operation($field_info, $column, $val);
        }
      }
      else {
        // Just pick the first field column for the operation.
        $columns = array_keys($field_info['columns']);
        $column = $columns[0];
        $query->$operation($field_info, $column, $value);
      }
    }
  }

}

<?php

class PubApiResourceController
  extends RestWSEntityResourceController
  implements RestWSQueryResourceControllerInterface {

  protected $apiName, $apiMap, $apiSpec, $bundleName;

  public function __construct($name, $info) {
    $this->apiMap = pubapi_get_map();
    $this->apiSpec = pubapi_get_structure();
    $this->apiName = $name;

    $this->entityType = $this->apiMap[$name]['entity'];
    $this->bundleName = $this->apiMap[$name]['bundle'];
    $this->entityInfo = entity_get_info($this->apiMap[$name]['entity']);
  }

  public function propertyInfo() {
    return $this->apiSpec;
  }

  /**
   * @see RestWSResourceControllerInterface::wrapper()
   */
  public function wrapper($id) {
    $info = $this->apiSpec[$this->apiName];
    $object = $this->objectLoad($id);

    return entity_metadata_wrapper($this->apiName, $object, array('property info' => $info['properties']));
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
   */
  public function resource() {
    return $this->apiName;
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
    $object = new stdClass();

    // Get original entity.
    $info = $this->apiSpec[$this->apiName];
    $original_wrapper = $this->originalWrapper($id);

    // If the wrapped entity is not of the correct bundle, bail now.
    if ($original_wrapper->getBundle() !== $this->bundleName) {
      return $object;
    }

    $original_properties = $original_wrapper->getPropertyInfo();

    // Add to our object according to our defined API property info.
    if ($map = $this->apiMap[$this->apiName]) {
      foreach (array_keys($info['properties']) as $property) {
        if (array_key_exists($map[$property], $original_properties)) {
          $value = $original_wrapper->{$map[$property]}->value(array('sanitize' => TRUE));
          // For now make a quick check for references, and get just the ID.
          // @todo abstract this with a getter callback wrapper or something for
          //   entityreference.
          if (in_array($property, array('show', 'season', 'episode'))) {
            $value = isset($value->nid) ? $value->nid : NULL;
          }
          // If the value is an array, it's a filtered text field. Example for
          // body: $wrapper->body->value->value(array('decode' => TRUE));
          // @todo abstract this.
          $value = is_object($value) ? $value->value(array('decode' => TRUE)) : $value;
          $value = is_array($value) ? $original_wrapper->{$map[$property]}->value->value(array('decode' => TRUE)) : $value;
          $object->{$property} = $value;
        }
      }
    }

    return $object;
  }

}

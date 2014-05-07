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
   * @todo make an wrapper with any methods restws expects.
   */
  //public function wrapper($id) {}

  public function read($id) {
    $object = new stdClass();

    foreach($this->apiSpec[$this->apiName] as $key) {
      if (isset($this->apiMap[$key])) {
        $field = $this->apiMap[$key];
        $object->{$key} = $this->wrapper($id)->{$field}->value();
      }
    }

    return $object;
  }

}

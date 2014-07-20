RestWS Schema
=============

Installation
============

- Install and update the restws_schema Drupal 7 module [per usual]
  (https://drupal.org/node/895232) (ideally with Drush)
- Optionally enable restws_schema_ui module.

Modules
=======

RestWS Schema
-------------

An API-only module, which extends RESTful Web Services

- All resources can be accessed in a list by resource type. Examples:
    - URI: RESOURCE.json
    - Contains: The 'list' key is an array of individual resource objects.
- Each invdividual resource can be accessed directly. Example:
  - URI: RESOURCE/ID.json
  - Object keys are defined in the spec, including:
    - id: The current resource object ID
  - References (including corresponding references) are entityreference fields,
    and contain:
    - uri: The referenced resource object REST URI
    - resource: The referenced resource name
    - id: The referenced resource object ID

RestWS Schema UI
----------------

A bundle/property mapper, and color-coded status alert, for restws_schema.

- Explore the mapper
    - URL: admin/config/services/restws_schema
- Check out the status
    - URL: admin/reports/status (where it should be, with the other color-coded
      requirements reports)
    - Status page should display a red error: "The RestWS Schema is not
      satisfied. Configure here."
    - Additionally the main config page (admin/config) will display an error:
      "One or more problems were detected with your Drupal installation. Check
      the status report for more information."
- See it work:
    - After mapping the restws_schema keys to Drupal entity equivalents, this
      should be green, and say: "The RestWS Schema is satisfied."

Dependencies
============

- RESTful Web Services (depends on Entity API)

Recommended
===========

- CORS (makes JS cross-origin resource sharing work, for example AngularJS)
- Entity reference (for references)
- Corresponding Entity References (reverse references)

Contributors
============

- [scottrigby](https://drupal.org/u/scottrigby)

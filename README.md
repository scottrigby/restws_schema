    o--o       o          O  o--o  o-O-o
    |   |      |         / \ |   |   |
    O--o  o  o O-o      o---oO--o    |
    |     |  | |  |     |   ||       |
    o     o--o o-o      o   oo     o-O-o

Installation
============

- Install and update the PubAPI Drupal 7 module [per usual]
  (https://drupal.org/node/895232) (ideally with Drush)
- Optionally enable pubapi_example feature.

Modules
=======

PubAPI
------

An API-only module, which extends RESTful Web Services

- All resources can be accessed in a list by resource type. Examples:
    - URI: RESOURCE.json
    - Contains: The 'list' key is an array of individual resource objects.
- Each invdividual resource can be accessed directly. Example for 'Show':
  - URI: show/ID.json
  - Object keys are defined in the spec, including:
    - id: The current resource object ID
  - References (including corresponding references) are entityreference fields,
    and contain:
    - uri: The referenced resource object REST URI
    - resource: The referenced resource name (example: 'season')
    - id: The referenced resource object ID

PubAPI example
--------------

A feature for demo purposes only

1. Enable 'pubapi_example' module (it will enable all necessary dependencies)
2. Generate some dummy content in this order (so the entityreference fields have
   content to reference):
    - `drush generate-content 5 --kill --types=page -y`
    - `drush generate-content 5 --kill --types=season -y`
    - `drush generate-content 5 --kill --types=episode -y`
    - `drush generate-content 5 --kill --types=article -y`
    - `drush generate-content 5 --kill --types=gallery -y`
    - NOTE: The example module maps 'Page' to the 'Show' object in our API (as
      an arbitrary example that any bundle can be used)
3. See results
    - Browse to our API's Show URI: show.json, show/ID.json
    - Now repeat for Season, Episode, and Blog
    - Follow the referenced resources via the URI key


PubAPI UI
---------

A bundle/property mapper, and color-coded status alert, for PubAPI

- Explore the mapper
    - URL: admin/config/services/pubapi
    - NOTE: this is built dynamically from a mock-up API schema (temporarily
      defined in pubapi_get_structure(). We'll replace that with our final
      schema once that's complete)
- Check out the status
    - URL: admin/reports/status (where it should be, with the other color-coded
      requirements reports)
- See it work:
    - After enabling the pubapi_example module, this will always be green, and
      say: "The Publisher API is satisfied."
- See it break:
    - Disable the pubapi_example module: `drush dis pubapi_example -y`
    - Delete the variables added by pubapi_example module, which satisfied the
      mapping: `drush sqlq "delete from variable where name like 'pubapi%'"`
    - Clear the caches `drush cc all`
    - Status page should display a red error: "The Publisher API is not
      satisfied. Configure here."
    - Additionally the main config page (admin/config) will display an error:
      "One or more problems were detected with your Drupal installation. Check
      the status report for more information."

Dependencies
============

- PubAPI
    - RESTful Web Services (depends on Entity API)
- PubAPI UI
    - PubAPI
- PubAPI example
    - PubAPI
    - CTtools (for exportables)
    - Devel (to generate dummy content)
    - Entity reference (for references)
    - Features (to store exportables)
    - Strongarm (to store example configs)

Credits
=======

- The NBCU O&TS Developer Working Group

<?php
/**
 * @file
 * pubapi_example.cer.inc
 */

/**
 * Implements hook_cer_default_presets().
 */
function pubapi_example_cer_default_presets() {
  $export = array();

  $preset = new CerPreset();
  $preset->disabled = FALSE; /* Edit this to true to make a default preset disabled initially */
  $preset->a = 'node:season:field_episodes';
  $preset->b = 'node:episode:field_season';
  $preset->bidirectional = TRUE;
  $preset->weight = 0;
  $export['6976f7bd5d1933daf0a92f6a7015ae75'] = $preset;

  $preset = new CerPreset();
  $preset->disabled = FALSE; /* Edit this to true to make a default preset disabled initially */
  $preset->a = 'node:page:field_seasons';
  $preset->b = 'node:season:field_show';
  $preset->bidirectional = TRUE;
  $preset->weight = 0;
  $export['ad83632b6ce24f0558bbe6cba993a0a4'] = $preset;

  return $export;
}

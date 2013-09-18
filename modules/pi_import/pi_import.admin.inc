<?php

/**
 * @file
 * Contains all admin form related code.
 */


/**
 * Page callback for admin/content/percolate. Provides a form for importing
 * Percolate content.
 */
function pi_import_form($form, &$form_state) {
  $form = array();
  $default = pi_import_settings();
  
  // Import settings
  $form['import_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Import settings'),
  );
  
  $form['import_settings']['import_type'] = array(
    '#type' => 'select',
    '#title' => t('Import type'),
    '#options' => array(
      PI_IMPORT_TYPE_LICENSE => t('License'),
      PI_IMPORT_TYPE_USER => t('User'),
    ),
    '#default_value' => $default['import_type'],
  );
  
  $form['import_settings']['import_id'] = array(
    '#type' => 'textfield',
    '#title' => t('ID'),
    '#required' => TRUE,
    '#description' => t('ID of the type as specified above to import from.'),
    '#default_value' => $default['import_id'],
    '#size' => 13,
  );
  
  $form['import_settings']['import_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of articles to import'),
    '#description' => t('Choose how many of the latest posts to import. Duplicates will not be imported. Reducing this number reduces import time, but may cause you to miss posts since the last import.'),
    '#default_value' => $default['import_count'],
    '#size' => 3,
  );
  
  
  // Field mappings
  $types = node_type_get_types();
  $options = array();
  foreach ($types as $type) {
    $options[$type->type] = $type->name;
  }
  $form['content_type'] = array(
    '#type' => 'select',
    '#title' => t('Content type'),
    '#required' => TRUE,
    '#description' => t('The content type you want to map the percolate post to.'),
    '#options' => $options,
    '#default_value' => $default['content_type'],
    '#empty_option' => t('- Select -'),
    '#ajax' => array(
      'callback' => 'pi_import_form_content_type_ajax',
      'wrapper' => 'field-mappings',
    ),
  );

  $form['field_mappings'] = pi_import_build_field_mappings_table($default['content_type'], $default);
  
  
  // Node settings
  $form['node_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Node settings'),
  );
  
  $user = user_load($default['default_author']);
  $form['node_settings']['default_author'] = array(
    '#type' => 'textfield',
    '#title' => t('Default author'),
    '#description' => t("Set the default author that content should be published by if the percolate post author ID isn't matched to a drupal user ID in the !mapping. Leave blank for anonymous.", array(
      '!mapping' => l('mapping form', 'admin/config/content/percolate/users'),
    )),
    '#default_value' => $user ? $user->name : '',
    '#size' => 24,
    '#autocomplete_path' => 'user/autocomplete',
  );

  $formats = filter_formats();
  $format_options = array();
  foreach ($formats as $format) {
    $format_options[$format->format] = $format->name;
  }
  $form['node_settings']['text_format'] = array(
    '#type' => 'select',
    '#title' => t('Body text format'),
    '#description' => t('Text format the body should be rendered with. Keep in mind that Percolate content may include HTML markup.'),
    '#options' => $format_options,
    '#default_value' => $default['text_format']
  );
  
  $form['node_settings']['match_publish_date'] = array(
    '#type' => 'checkbox',
    '#title' => t('Match post publish date'),
    '#description' => t('Sets the node publish date to the percolate post publish date'),
    '#default_value' => $default['match_publish_date']
  );
  
  $form['node_settings']['reimport_deleted'] = array(
    '#type' => 'checkbox',
    '#title' => t('Re-import deleted'),
    '#description' => t('Import posts that were imported, and then deleted in Drupal again.'),
    '#default_value' => $default['reimport_deleted'],
  );
  
  $form['node_settings']['publish'] = array(
    '#type' => 'checkbox',
    '#title' => t('Publish imported nodes'),
    '#default_value' => $default['publish'],
  );
  
  // Action buttons
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('pi_import_form_save_settings'),
  );
  $form['actions']['import'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#submit' => array(
      'pi_import_form_save_settings',
      'pi_import_form_import',
    ),
  );

  $form['#validate'][] = 'pi_import_form_validate';
  $form['#attached']['css'][] = drupal_get_path('module', 'pi_import') . '/css/pi_import.css';
  
  return $form;
}


/**
 * Validate callback for pi_import_form.
 */
function pi_import_form_validate($form, &$form_state) {
  // Check that default author is either a real user, or empty.
  if (empty($form_state['values']['default_author'])) {
    $form_state['values']['default_author'] = 0;
  }
  else {
    $user = user_load_by_name($form_state['values']['default_author']);
    if ($user) {
      $form_state['values']['default_author'] = $user->uid;
    }
    else {
      form_set_error('default_author', t($form_state['values']['default_author'] . ' is not a valid username.'));
    }
  }
}


/**
 * Submit callback for pi_import_form. Saves settings to variable.
 */
function pi_import_form_save_settings($form, &$form_state) {
  $settings = pi_import_settings();
  foreach ($settings as $name => $setting) {
    $settings[$name] = $form_state['values'][$name];
  }
  variable_set(PI_IMPORT_SETTINGS, $settings);
}

/**
 * Provide defaults for the import form.
 */
function pi_import_settings() {
  return array_merge(array(
    'import_type' => PI_IMPORT_TYPE_LICENSE,
    'import_id' => '',
    'import_count' => 20,
    'content_type' => '',
    'field_mappings' => array(),
    'default_author' => 0,
    'match_publish_date' => TRUE,
    'reimport_deleted' => TRUE,
    'publish' => FALSE,
    'text_format' => 'filtered_html',
  ), variable_get(PI_IMPORT_SETTINGS, array()));
}


/**
 * Build the table that allows a user to specify the maping of percolate content
 * to node fields.
 */
function pi_import_build_field_mappings_table($content_type, $default) {
  $types = node_type_get_types();
  $form = array(
    '#prefix' => '<div id="field-mappings">',
    '#type' => 'fieldset',
    '#title' => t('Post to node field mapping'),
    '#description' => t('Select how you want percolate post fields to map to node fields. Title and author will automatically be mapped to the post title, and Percolate UID (Which can be mapped to drupal users !here) respectively. Not all fields can be mapped to, only link fields, text-based fields, term fields, and file fields.', array(
      '!here' => l('here', 'admin/config/content/percolate/users'),
    )),
    '#tree' => TRUE,
    '#required' => TRUE,
    '#suffix' => '</div>',
  );
  
  if ($content_type) {
    $post_fields = pi_get_post_fields();
    $node_fields = pi_get_content_type_fields($content_type);
    
    foreach ($node_fields as $name => $field) {
      $value = isset($default['field_mappings'][$content_type][$name]) ? $default['field_mappings'][$content_type][$name] : 0;
      $form[$content_type][$name] = array(
        '#type' => 'select',
        '#title' => $field['label'],
        '#options' => pi_get_field_post_field_options($field, $post_fields),
        '#required' => empty($field['required']) ? FALSE : TRUE,
        '#default_value' => $value,
      );
    }
  }
  else {
    $form['no_fields'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="description">',
      '#markup' => t('Select a content to specify field mappings.'),
      '#suffix' => '</div>',
    );
  }
  
  return $form;
}


/**
 * Returns an array that defines what fields from a percolate post that this
 * module is aware of, and what it knows how to map.
 */
function pi_get_post_fields() {
  $text = array('text', 'text_long', 'text_with_summary');
  $int = array('text', 'text_long', 'text_with_summary', 'number_integer');
  $file = array('file', 'image');
  $terms = array('taxonomy_term_reference');
  $link = array('text', 'text_long', 'text_with_summary', 'link_field');
  
  return array(
    'title' => array(
      'label' => 'Title',
      'types' => $text,
    ),
    'body' => array(
      'label' => 'Body',
      'types' => $text,
    ),
    'media' => array(
      'label' => 'Attached media',
      'types' => $file,
    ),
    'created_at' => array(
      'label' => 'Create date',
      'types' => $int,
    ),
    'tags' => array(
      'label' => 'Tags',
      'types' => $terms,
    ),
    'link_url' => array(
      'label' => 'Link - URL',
      'types' => $link,
    ),
    'link_title' => array(
      'label' => 'Link - Title',
      'types' => $text,
    ),
    'link_description' => array(
      'label' => 'Link - Description',
      'types' => $text,
    ),
    'short_url' => array(
      'label' => 'Shortened Percolate URL',
      'types' => $link,
    ),
    'id' => array(
      'label' => 'Percolate post ID',
      'types' => $int,
    ),
  );
}


/**
 * Return the list of fields that are applicable to a node field from a post.
 */
function pi_get_field_post_field_options($node_field, $post_fields) {
  $fields = array();

  // FAPI #empty_option doesn't work with AJAX, so handle it here
  $fields[0] = '- None -';

  foreach ($post_fields as $name => $post_field) {
    if (in_array($node_field['type'], $post_field['types'])) {
      $fields[$name] = $post_field['label'];
    }
  }
  
  return $fields;
}


/**
 * Return an array of fields that pi_import can handle. Complicated fields
 * are broken out in to multiple fields.
 */
function pi_get_content_type_fields($content_type) {
  $field_info = field_info_fields();
  $node_fields = field_info_instances('node', $content_type);
  $allowed_field_types = array(
    'text',
    'text_long',
    'text_with_summary',
    'image',
    'file',
    'link_field',
    'number_integer',
    'taxonomy_term_reference',
  );
  
  // Break out links in to multiple fields.
  $fields = array();
  foreach ($node_fields as $name => $field) {
    if (in_array($field_info[$name]['type'], $allowed_field_types)) {
      // Split link fields out in to multiple.
      if ($field_info[$name]['type'] == 'link_field') {
        $fields[$name . ':url'] = array(
          'type' => 'link_field',
          'label' => $field['label'] . ' - URL',
          'required' => $field['required'],
          'field_name' => $name,
        );
        if ($field['settings']['title'] != 'none') {
          $fields[$name . ':title'] = array(
            'type' => 'text',
            'label' => $field['label'] . ' - Title',
            'required' => ($field['required'] && $field['settings']['title'] == 'required') ?
              TRUE :
              FALSE,
            'field_name' => $name,
          );
        }
      }
      else {
        $fields[$name] = array(
          'type' => $field_info[$name]['type'],
          'label' => $field['label'],
          'required' => $field['required'],
          'field_name' => $name,
        );
      }
    }
  }
  
  
  return $fields;
}

/**
 * AJAX callback that replaces the mappings table with the content type's fields
 */
function pi_import_form_content_type_ajax($form, &$form_state) {
  $default = pi_import_settings();
  return pi_import_build_field_mappings_table($form_state['values']['content_type'], $default);
}



/**
 * Page callback for admin/config/content/percolate/import. Provides settings
 * form for imports. 
 */
function pi_import_settings_form($form, &$form_state) {
  $form['pi_import_automatic_import'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically import content from Percolate.'),
    '#description' => t('Check this to import content automatically from Percolate using the settings configured at the !form.', array(
      '!form' => l('import form', 'admin/content/percolate'),
    )),
    '#default_value' => variable_get('pi_import_automatic_import', FALSE),
  );
  
  $cron_threshold = variable_get('cron_safe_threshold', DRUPAL_CRON_DEFAULT_THRESHOLD) / 60;
  $form['pi_import_threshold'] = array(
    '#title' => t('Import frequency'),
    '#description' => t('Import can no more often than cron, which has been configured to run every ' . $cron_threshold . ' minutes.'),
    '#type' => 'textfield',
    '#size' => 5,
    '#field_prefix' => 'Import every',
    '#default_value' => variable_get('pi_import_threshold', PI_IMPORT_DEFAULT_FREQUENCY) / 60,
    '#field_suffix' => 'minutes',
  );
  
  $form['#validate'][] = 'pi_import_settings_form_validate';
  
  return system_settings_form($form);
}


/**
 * Validate callback for pi_import_settings_form.
 */
function pi_import_settings_form_validate($form, &$form_state) {
  if (is_numeric($form_state['values']['pi_import_threshold'])) {
    $form_state['values']['pi_import_threshold'] *= 60;
  }
  else {
    form_set_error('pi_import_threshold', t('Import frequency must be a number.'));
  }
}

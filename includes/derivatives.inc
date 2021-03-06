<?php

/**
 * @file
 * This file contains functions used to generate derivatives of audio objects.
 */

/**
 * Creates the PROXY_MP3 derivative from the given audio file.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object in Fedora.
 *
 * @param bool $force
 *   Whether the derivative generation is being forced.
 *
 * @return array
 *   An array describing the outcome of the PROXY_MP3 creation.
 *
 * @see hook_islandora_derivative()
 */
function islandora_audio_create_mp3(AbstractObject $object, $force = FALSE) {
  if (!isset($object['PROXY_MP3']) || (isset($object['PROXY_MP3']) && $force === TRUE)) {
    if (!isset($object["OBJ"])) {
      return array(
        'success' => FALSE,
        'messages' => array(
          array(
            'message' => t('No OBJ datastream present for object @pid - PROXY_MP3 creation was skipped.'),
            'message_sub' => array('@pid' => $object->id),
            'type' => 'watchdog',
            'severity' => WATCHDOG_ERROR,
          ),
        ),
      );
    }
    else {
      $mime_detect = new MimeDetect();
      $ext = $mime_detect->getExtension($object['OBJ']->mimeType);
      $file_name = str_replace(":", "-", $object->id) . ".OBJ.{$ext}";
      $file_uri = file_create_filename($file_name, 'temporary://');
      $object['OBJ']->getContent($file_uri);
      $output = array();
      $file = drupal_realpath($file_uri);
      $outfile = $file . '.mp3';
      $lame_url = variable_get('islandora_lame_url', '/usr/bin/lame');
      $command = "$lame_url -V5 --vbr-new '${file}' '${outfile}'";
      $ret = FALSE;
      exec($command, $output, $ret);
      // Successfully crafted the derivative.
      if ($ret == '0') {
        return islandora_audio_add_datastream($object, 'PROXY_MP3', $outfile);
      }
      // Failure.
      else {
        return array(
          'success' => FALSE,
          'messages' => array(
            array(
              'message' => t('LAME encoding failed for object @pid - PROXY_MP3 creation was skipped.'),
              'message_sub' => array('@pid' => $object->id),
              'type' => 'watchdog',
              'severity' => WATCHDOG_ERROR,
            ),
          ),
        );
      }
    }
  }
}

/**
 * Creates the TN derivative for the audio object.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object in Fedora.
 *
 * @param bool $force
 *   Whether the derivative generation is being forced.
 *
 * @return array
 *   An array describing the outcome of the PROXY_MP3 creation.
 *
 * @see hook_islandora_derivative()
 */
function islandora_audio_create_thumbnail(AbstractObject $object, $force = FALSE) {
  if (!isset($object['TN']) || (isset($object['TN']) && $force === TRUE)) {
    if (!isset($object["OBJ"])) {
      return array(
        'success' => FALSE,
        'messages' => array(
          array(
            'message' => t('No OBJ datastream present for object @pid - PROXY_MP3 creation was skipped.'),
            'message_sub' => array('@pid' => $object->id),
            'type' => 'watchdog',
            'severity' => WATCHDOG_ERROR,
          ),
        ),
      );
    }
    else {
      return islandora_audio_add_datastream($object, 'TN', drupal_get_path('module', 'islandora_audio') . '/images/audio-TN.jpg');
    }
  }
}
/**
 * Adds the given file as a datastream to the given object.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 * @param string $datastream_id
 *   The datastream id of the added datastream.
 * @param string $file_uri
 *   A URI to the file containing the content for the datastream.
 *
 * @return array
 *   An array describing the outcome of the datastream addition.
 */
function islandora_audio_add_datastream(AbstractObject $object, $datastream_id, $file_uri) {
  try {
    $ingest = !isset($object[$datastream_id]);
    $mime_detector = new MimeDetect();

    if ($ingest) {
      $ds = $object->constructDatastream($datastream_id, "M");
      $ds->label = $datastream_id;
    }
    else {
      $ds = $object[$datastream_id];
    }
    $ds->mimetype = $mime_detector->getMimetype($file_uri);
    $ds->setContentFromFile(drupal_realpath($file_uri));
    if ($ingest) {
      $object->ingestDatastream($ds);
    }
    return array(
      'success' => TRUE,
      'messages' => array(
        array(
          'message' => t('Created @dsid derivative for OBJ (@pid).'),
          'message_sub' => array(
            '@dsid' => $datastream_id,
            '@pid' => $object->id,
          ),
          'type' => 'dsm',
        ),
      ),
    );
  }
  catch (exception $e) {
    return array(
      'success' => FALSE,
      'messages' => array(
        array(
          'message' => t('Failed to add @dsid datastream for @pid. Error message: @message'),
          'message_sub' => array(
            '@dsid' => $datastream_id,
            '@pid' => $object->id,
            '@message' => $e->getMessage(),
          ),
          'type' => 'watchdog',
          'severity' => WATCHDOG_ERROR,
        ),
      ),
    );
  }
}

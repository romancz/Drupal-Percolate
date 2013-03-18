<?php

require_once dirname(__FILE__) . '/Exception.php';

class Percolate_Api {
  /**
   * The default endpoint URL for the percolate API.
   *
   * @var string
   */
  const API_URL = 'http://percolate.com/api/v3/';
  /**
   * Number of seconds to wait until timeout.
   *
   * @var integer
   */
  const TIMEOUT = 10;
  
  
  /**
   * Percolate API key.
   *
   * @var string
   */
  private $key = '';
  
  /**
   * Constructor.
   *
   * @param string $key - Percolate API key.
   */
  public function __construct($key) {
    $this->key = $key;
  }
  
  
  
  /*********/
  /* Users */
  /*********/
  
  public function getUser($user_id) {
    return $this->executeMethod('users/' . $user_id);
  }
  
  
  /*********/
  /* Posts */
  /*********/
  
  
  public function getPost($post_id) {
    $results = $this->executeMethod('posts/' . $post_id);
  }
  
  public function getUserPosts($user_id, $limit) {
    return $this->executeMethod('users/' . $user_id . '/posts', array(
      'limit' => $limit,
    ));
  }
  
  
  /***********/
  /* Utility */
  /***********/
  
  
  /**
   * Execute an API method.
   *
   * @param string $method - API method.
   * @param array  $params - Array of parameters.
   * @param string $type   - HTTP method type to use.
   *
   * @throws Percolate_ConnectionException
   *
   * @return array
   */
  public function executeMethod($method, $params = array(), $type = 'GET') {
    $params['api_key'] = $this->key;
    $url = self::API_URL . $method;
  
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $type);
    curl_setopt($ch, CURLOPT_TIMEOUT, self::TIMEOUT);
    curl_setopt($ch, CURLOPT_POSTFIELDS, $this->arrayToParams($params));
    curl_setopt($ch, CURLOPT_FAILONERROR, 1);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    
    $results = curl_exec($ch);
    if (curl_errno($ch) == 0) {
      curl_close($ch);
      $results = json_decode($results, TRUE);
      return $results;
    }
    else {
      throw new Percolate_Exception(curl_error($ch), curl_errno($ch), $url);
    }
  }
  
  /**
   * Converts a keyed array of parameters in to a URL friendly list of params.
   *
   * @param array $params - Keyed array of parameters.
   *
   * @return string
   */
  private function arrayToParams($params) {
    $param_str = '';
    foreach ($params as $key => $value) {
      $param_str .= $key . '=' . $value . '&'; 
    }
    return $param_str;
  }
}
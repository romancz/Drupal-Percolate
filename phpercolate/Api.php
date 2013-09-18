<?php

// Service IDs used in arguments. Undocumented.
define('PERCOLATE_SERVICE_ID_PUBLIC', 12);

require_once dirname(__FILE__) . '/Exception.php';

class PercolateApi {
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
  
  /**
   * Gets a user by their Percolate user ID.
   *
   * @param int $user_id - Percolate user ID.
   *
   * @return array
   */
  public function getUser($user_id) {
    return $this->executeMethod('users/' . $user_id);
  }
  
  
  /*********/
  /* Posts */
  /*********/
  
  /**
   * Gets a single post by post id.
   *
   * @param int $post_id - Percolate post ID.
   *
   * @return array
   */
  public function getPost($post_id) {
    $results = $this->executeMethod('posts/' . $post_id);
  }
  
  /**
   * Gets a list of a user's posts.
   *
   * @param int $user_id - Percolate user ID.
   * @param array $options - Array of options for filtering / sorting. See
   *    Percolate API docs for more information.
   *
   * @return array - 'pagination' and 'data' indexes, 'data' contains the
   *    posts, 'pagination' contains info about the query.
   */
  public function getUserPosts($user_id, $options = array()) {
    return $this->executeMethod('users/' . $user_id . '/posts', $options);
  }
  
  
  /**********/
  /* Groups */
  /**********/
  
  /**
   * Gets a list of users in a group.
   *
   * @param int $group_id - Group ID.
   * @param array $options - Array of options for filtering / sorting. See the
   *    Percolate API docs for more information.
   *
   * @return array
   */
  public function getGroupUsers($group_id, $options = array()) {
    if (!isset($options['limit'])) {
      $options['limit'] = 9999; // Defaults to 10, and we want to get all.
    }
    $users = $this->executeMethod('groups/' . $group_id . '/users', $options);
    return $users['data'];
  }


  /**
   * Gets a list of posts made by users in a group.
   *
   * @param int   $group_id - Group ID.
   * @param array $options - Array of options. See getUserPosts for more info.
   *
   * @return array
   *
   * @see getUserPosts
   */
  public function getGroupPosts($group_id, $options = array()) {
    return $this->executeMethod('groups/' . $group_id . '/posts', $options);
  }


  /************/
  /* Licenses */
  /************/

  public function getLicensePosts($license_id, $options = array()) {
    return $this->executeMethod('licenses/' . $license_id . '/posts', $options);
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
    if ($type == 'GET') {
      $url .= '?' . $this->arrayToParams($params);
    }

    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $type);
    curl_setopt($ch, CURLOPT_TIMEOUT, self::TIMEOUT);
    curl_setopt($ch, CURLOPT_FAILONERROR, 1);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

    if ($type == 'POST') {
      curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
    }

    $results = curl_exec($ch);
    if (curl_errno($ch) == 0) {
      curl_close($ch);
      $results = json_decode($results, TRUE);
      return $results;
    }
    else {
      throw new PercolateException(
        curl_error($ch),
        curl_getinfo($ch, CURLINFO_HTTP_CODE),
        $url,
        curl_errno($ch)
      );
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
      $param_str .= $key . '=' . urlencode($value) . '&'; 
    }
    return $param_str;
  }
}

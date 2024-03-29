<?php
namespace MRBS;

define('LDAP_SUCCESS',                        0x00);
define('LDAP_OPERATIONS_ERROR',               0x01);
define('LDAP_PROTOCOL_ERROR',                 0x02);
define('LDAP_TIMELIMIT_EXCEEDED',             0x03);
define('LDAP_SIZELIMIT_EXCEEDED',             0x04);
define('LDAP_COMPARE_FALSE',                  0x05);
define('LDAP_COMPARE_TRUE',                   0x06);
define('LDAP_AUTH_METHOD_NOT_SUPPORTED',      0x07);
define('LDAP_STRONG_AUTH_REQUIRED',           0x08);
// Not used in LDAPv3
define('LDAP_PARTIAL_RESULTS',                0x09);

// Next 5 new in LDAPv3
define('LDAP_REFERRAL',                       0x0a);
define('LDAP_ADMINLIMIT_EXCEEDED',            0x0b);
define('LDAP_UNAVAILABLE_CRITICAL_EXTENSION', 0x0c);
define('LDAP_CONFIDENTIALITY_REQUIRED',       0x0d);
define('LDAP_SASL_BIND_INPROGRESS',           0x0e);

define('LDAP_NO_SUCH_ATTRIBUTE',              0x10);
define('LDAP_UNDEFINED_TYPE',                 0x11);
define('LDAP_INAPPROPRIATE_MATCHING',         0x12);
define('LDAP_CONSTRAINT_VIOLATION',           0x13);
define('LDAP_TYPE_OR_VALUE_EXISTS',           0x14);
define('LDAP_INVALID_SYNTAX',                 0x15);

define('LDAP_NO_SUCH_OBJECT',                 0x20);
define('LDAP_ALIAS_PROBLEM',                  0x21);
define('LDAP_INVALID_DN_SYNTAX',              0x22);
// Next two not used in LDAPv3',
define('LDAP_IS_LEAF',                        0x23);
define('LDAP_ALIAS_DEREF_PROBLEM',            0x24);

define('LDAP_INAPPROPRIATE_AUTH',             0x30);
define('LDAP_INVALID_CREDENTIALS',            0x31);
define('LDAP_INSUFFICIENT_ACCESS',            0x32);
define('LDAP_BUSY',                           0x33);
define('LDAP_UNAVAILABLE',                    0x34);
define('LDAP_UNWILLING_TO_PERFORM',           0x35);
define('LDAP_LOOP_DETECT',                    0x36);

define('LDAP_SORT_CONTROL_MISSING',           0x3C);
define('LDAP_INDEX_RANGE_ERROR',              0x3D);

define('LDAP_NAMING_VIOLATION',               0x40);
define('LDAP_OBJECT_CLASS_VIOLATION',         0x41);
define('LDAP_NOT_ALLOWED_ON_NONLEAF',         0x42);
define('LDAP_NOT_ALLOWED_ON_RDN',             0x43);
define('LDAP_ALREADY_EXISTS',                 0x44);
define('LDAP_NO_OBJECT_CLASS_MODS',           0x45);
define('LDAP_RESULTS_TOO_LARGE',              0x46);
// Next two for LDAPv3
define('LDAP_AFFECTS_MULTIPLE_DSAS',          0x47);
define('LDAP_OTHER',                          0x50);

// Used by some APIs
define('LDAP_SERVER_DOWN',                    0x51);
define('LDAP_LOCAL_ERROR',                    0x52);
define('LDAP_ENCODING_ERROR',                 0x53);
define('LDAP_DECODING_ERROR',                 0x54);
define('LDAP_TIMEOUT',                        0x55);
define('LDAP_AUTH_UNKNOWN',                   0x56);
define('LDAP_FILTER_ERROR',                   0x57);
define('LDAP_USER_CANCELLED',                 0x58);
define('LDAP_PARAM_ERROR',                    0x59);
define('LDAP_NO_MEMORY',                      0x5a);

// Preliminary LDAPv3 codes
define('LDAP_CONNECT_ERROR',                  0x5b);
define('LDAP_NOT_SUPPORTED',                  0x5c);
define('LDAP_CONTROL_NOT_FOUND',              0x5d);
define('LDAP_NO_RESULTS_RETURNED',            0x5e);
define('LDAP_MORE_RESULTS_TO_RETURN',         0x5f);
define('LDAP_CLIENT_LOOP',                    0x60);
define('LDAP_REFERRAL_LIMIT_EXCEEDED',        0x61);


/* authLdapAction($callback, $user, &$object)
 * 
 * Connects/binds to all configured LDAP servers/base DNs and
 * then performs a callback, passing the LDAP object, $base_dn,
 * user DN (in $dn), $user and a generic object $object
 *
 * $callback - The callback function
 * $user     - The user name
 * &$object  - Reference to the generic object, type defined by caller
 * 
 * Returns:
 *   false    - The pair are invalid or do not exist
 *   string   - The validated username
 */
function authLdapAction($callback, $user, &$object)
{
  global $auth;
  global $ldap_host;
  global $ldap_port;
  global $ldap_v3;
  global $ldap_tls;
  global $ldap_base_dn;
  global $ldap_user_attrib;
  global $ldap_dn_search_attrib;
  global $ldap_dn_search_dn;
  global $ldap_dn_search_password;
  global $ldap_filter;
  global $ldap_group_member_attrib;
  global $ldap_admin_group_dn;
  global $ldap_email_attrib;
  global $ldap_name_attrib;
  global $ldap_disable_referrals;
  global $ldap_deref;
  global $ldap_filter_base_dn;
  global $ldap_filter_user_attr;

  if (!function_exists("ldap_connect"))
  {
    die("<hr><p><b>ERROR: PHP's 'ldap' extension is not installed/enabled. ".
        "Please check your MRBS and web server configuration.</b></p><hr>\n");
  }

  $ns_callback = __NAMESPACE__ . "\\$callback";
  
  // Transfer the values from the config variables into a local
  // associative array, turning them all into arrays
  
  $config_items = array(
                        'ldap_host',
                        'ldap_port',
                        'ldap_base_dn',
                        'ldap_user_attrib',
                        'ldap_dn_search_attrib',
                        'ldap_dn_search_dn',
                        'ldap_dn_search_password',
                        'ldap_filter',
                        'ldap_group_member_attrib',
                        'ldap_admin_group_dn',
                        'ldap_v3',
                        'ldap_tls',
                        'ldap_email_attrib',
                        'ldap_name_attrib',
                        'ldap_disable_referrals',
                        'ldap_deref',
                        'ldap_filter_base_dn',
                        'ldap_filter_user_attr'
                       );

  $all_ldap_opts = array();

  
  foreach ($config_items as $item)
  {
    if (!isset($$item))
    {
      continue;
    }
    if (is_array($$item))
    {
      $all_ldap_opts[$item] = $$item;
    }
    // The case where the config item _isn't_ an array is handled
    // further down
  }

  $count = null;
  foreach ($all_ldap_opts as $key => $value)
  {
    if (isset($count))
    {
      if (count($value) != $count)
      {
        authLdapDebug("Count of LDAP array config variables doesn't match, aborting!");
        fatal_error("MRBS configuration error: Count of LDAP array config variables doesn't match, aborting!");
        return false;
      }
    }
    $count = count($value);
  }

  // Turn any non-array config items into arrays in $all_ldap_opts
  if (!isset($count))
  {
    $count = 1;
  }
  foreach ($config_items as $item)
  {
    if (!isset($$item))
    {
      continue;
    }
    if (!is_array($$item))
    {
      $all_ldap_opts[$item] = array_fill(0, $count, $$item);
    }
  }

  foreach ($all_ldap_opts['ldap_host'] as $idx => $host)
  {
    // establish ldap connection
    if (isset($all_ldap_opts['ldap_port'][$idx]))
    {
      $ldap = ldap_connect($host, $all_ldap_opts['ldap_port'][$idx]);
    }
    else
    {
      $ldap = ldap_connect($host);
    }

    // Check that connection was established
    if ($ldap)
    {
      authLdapDebug("authLdapAction: Got LDAP connection");

      if (isset($all_ldap_opts['ldap_deref'][$idx]))
      {
        ldap_set_option($ldap, LDAP_OPT_DEREF, $all_ldap_opts['ldap_deref'][$idx]);
      }
      if (isset($all_ldap_opts['ldap_v3'][$idx]) &&
          $all_ldap_opts['ldap_v3'][$idx])
      {
        ldap_set_option($ldap, LDAP_OPT_PROTOCOL_VERSION, 3);
      }
      if (isset($all_ldap_opts['ldap_tls'][$idx]) &&
          $all_ldap_opts['ldap_tls'][$idx])
      {
        ldap_start_tls($ldap);
      }
      if(isset($all_ldap_opts['ldap_disable_referrals'][$idx]) && $all_ldap_opts['ldap_disable_referrals'][$idx])
      {
        // Required to do a search on Active Directory for Win 2003+
        ldap_set_option($ldap, LDAP_OPT_REFERRALS, 0);
      }
      
      if (isset($all_ldap_opts['ldap_dn_search_attrib'][$idx]))
      {
        if (isset($all_ldap_opts['ldap_dn_search_dn'][$idx]) &&
            isset($all_ldap_opts['ldap_dn_search_password'][$idx]))
        {
          // Bind with DN and password
          $res = mrbs_ldap_bind($ldap, $all_ldap_opts['ldap_dn_search_dn'][$idx],
                                $all_ldap_opts['ldap_dn_search_password'][$idx]);
        }
        else
        {
          // Anonymous bind
          $res = mrbs_ldap_bind($ldap);
        }

        if (!$res)
        {
          authLdapDebug("authLdapAction: Initial bind failed: ".ldap_error($ldap));
        }
        else
        {
          authLdapDebug("authLdapAction: Initial bind was successful");

          $res = ldap_search($ldap,
                             $all_ldap_opts['ldap_base_dn'][$idx],
                             "(" . $all_ldap_opts['ldap_dn_search_attrib'][$idx] . "=$user)");

          if (ldap_count_entries($ldap, $res) == 1)
          {
            authLdapDebug("authLdapAction: Found one entry using '".
                          $all_ldap_opts['ldap_dn_search_attrib'][$idx]."'");
            $entries = ldap_get_entries($ldap, $res);
            $dn = $entries[0]["dn"];
            $user_search = "distinguishedName=" . $dn;
          }
          else
          {
            authLdapDebug("authLdapAction: Didn't find entry using '".
                          $all_ldap_opts['ldap_dn_search_attrib'][$idx]."'");
          }
          authLdapDebug("authLdapAction: base_dn '".
                        $all_ldap_opts['ldap_base_dn'][$idx].
                        "' user '$user' dn '$dn'");
        }
      }
      else
      {
        // construct dn for user
        $user_search = $all_ldap_opts['ldap_user_attrib'][$idx] . "=" . $user;
        $dn = $user_search . "," . $all_ldap_opts['ldap_base_dn'][$idx];

        authLdapDebug("authLdapAction: Constructed dn '$dn' and ".
                      "user_search '$user_search' using '".
                      $all_ldap_opts['ldap_user_attrib'][$idx]."'");
      }

      foreach ($config_items as $item)
      {
        if (isset($all_ldap_opts[$item][$idx]))
        {
          $object['config'][$item] = $all_ldap_opts[$item][$idx];
        }
      }

      if (!$dn)
      {
        authLdapDebug("authLdapAction: No DN determined, not calling callback");
      }
      else
      {
        $res = $ns_callback($ldap, $all_ldap_opts['ldap_base_dn'][$idx], $dn,
                            $user_search, $user, $object);
        if ($res)
        {
          return $user;
        }
      }

    } // if ($ldap)

    ldap_unbind($ldap);
  } // foreach
  
  return false;
}


// Gets the user's email address.   Returns an empty
// string if one can't be found
function authGetUserEmail($user)
{
  global $ldap_get_user_email;
  
  if (!isset($user) || $user === '')
  {
    return '';
  }
  
  if ($ldap_get_user_email)
  {
    $email = authLdapGetEmail($user);
  }
  else
  {
    $email = get_default_email($user);
  }
  
  return $email;
}


/* authLdapGetEmail($user)
 * 
 * Gets the email address of the user from LDAP
 * 
 * $user  - The user name
 * 
 * Returns:
 *   The user's email address or ''
 */
function authLdapGetEmail($user)
{
  $email = '';
  $object = array();

  $res = authLdapAction("authLdapGetEmailCallback", $user, $object);

  if ($res)
  {
    $email = $object['email'];
  }
  return $email;
}


/* authLdapGetEmailCallback(&$ldap, $base_dn, $dn, $user_search,
                            $user, &$object)
 * 
 * Checks if the specified username/password pair are valid
 *
 * &$ldap       - Reference to the LDAP object
 * $base_dn     - The base DN
 * $dn          - The user's DN
 * $user_search - The LDAP filter to find the user
 * $user        - The user name
 * &$object     - Reference to the generic object
 * 
 * Returns:
 *   false    - Didn't find a user
 *   true     - Found a user
 */
function authLdapGetEmailCallback(&$ldap, $base_dn, $dn, $user_search,
                                  $user, &$object)
{
  $email_attrib = $object['config']['ldap_email_attrib'];

  authLdapDebug("authLdapGetEmailCallback: base_dn '$base_dn' dn '$dn' ".
                "user_search '$user_search' user '$user'");

  if ($ldap && $base_dn && $dn && $user_search)
  {
    $res = ldap_read($ldap,
                     $dn,
                     "(objectclass=*)",
                     array(utf8_strtolower($email_attrib)) );
                     
    if (ldap_count_entries($ldap, $res) > 0)
    {
      authLdapDebug("authLdapGetEmailCallback: search successful");
      $entries = ldap_get_entries($ldap, $res);
      $object['email'] = $entries[0][utf8_strtolower($email_attrib)][0];

      authLdapDebug("authLdapGetEmailCallback: email is '".
                    $object['email']."'");
      
      return true;
    }
  }
  return false;
}


/* authValidateUser($user, $pass)
 * 
 * Checks if the specified username/password pair are valid
 * 
 * $user  - The user name
 * $pass  - The password
 * 
 * Returns:
 *   false    - The pair are invalid or do not exist
 *   string   - The validated username
 */
function authValidateUser($user, $pass)
{
  // Check if we do not have a username/password
  // User can always bind to LDAP anonymously with empty password,
  // therefore we need to block empty password here...
  if (!isset($user) || !isset($pass) || strlen($pass)==0)
  {
    authLdapDebug("Empty username or password passed");
    return false;
  }

  $object = array();
  $object['pass'] = $pass;

  return authLdapAction("authValidateUserCallback", $user, $object);
}


/* authValidateUserCallback(&$ldap, $base_dn, $dn, $user_search,
                            $user, &$object)
 * 
 * Checks if the specified username/password pair are valid
 *
 * &$ldap       - Reference to the LDAP object
 * $base_dn     - The base DN
 * $dn          - The user's DN
 * $user_search - The LDAP filter to find the user
 * $user        - The user name
 * &$object     - Reference to the generic object
 * 
 * Returns:
 *   false      - Didn't find a user
 *   true       - Found a user
 */
function authValidateUserCallback(&$ldap, $base_dn, $dn, $user_search,
                                  $user, &$object)
{
  global $ldap_unbind_between_attempts;
  
  authLdapDebug("authValidateUserCallback: base_dn '$base_dn' ".
                "dn '$dn' user '$user'");

  $pass = $object['pass'];

  // try an authenticated bind
  // use this to confirm that the user/password pair
  if ($dn && mrbs_ldap_bind($ldap, $dn, $pass))
  {
    // however if there is a filter check that the
    // user is part of the group defined by the filter
    if (!isset($object['config']['ldap_filter']) || ($object['config']['ldap_filter'] === ''))
    {
      authLdapDebug("authValidateUserCallback: Successful authenticated ".
                    "bind with no \$ldap_filter");
      return true;
    }
    else
    {
      // If we've got a search DN and password, then bind again using those credentials because
      // it's possible that the user doesn't have read access in the directory, even for their own
      // entry, in which case we'll get a "No such object" result.
      if (isset($object['config']['ldap_dn_search_dn']) &&
          isset($object['config']['ldap_dn_search_password']))
      {
        authLdapDebug("authValidateUserCallback: rebinding as '" . $object['config']['ldap_dn_search_dn'] . "'");
        if (!mrbs_ldap_bind($ldap, $object['config']['ldap_dn_search_dn'], $object['config']['ldap_dn_search_password']))
        {
          authLdapDebug("authValidateUserCallback: rebinding failed: " . ldap_error($ldap));
          if ($ldap_unbind_between_attempts)
          {
            ldap_unbind($ldap);
          }
          return false;
        }
        authLdapDebug("authValidateUserCallback: rebinding successful");
      }

      $filter = $object['config']['ldap_filter'];
      
      authLdapDebug("authValidateUserCallback: Successful authenticated ".
                    "bind checking '$filter'");

      // If ldap_filter_base_dn is set, set the filter to search for the user
      // in the given base_dn (OpenLDAP).  If not, read from the user
      // attribute (AD)
      if (isset($object['config']['ldap_filter_base_dn']))
      {
        $f = "(&(".
          $object['config']['ldap_filter_user_attr'].
          "=$user)($filter))";
        $filter_dn = $object['config']['ldap_filter_base_dn'];
        $call = 'ldap_search';
      }
      else
      {
        $f = "($filter)";
        $filter_dn = $dn;
        $call = 'ldap_read';
      }

      authLdapDebug("authValidateUserCallback: Trying filter: $f: ".
                    "dn: $filter_dn: method: $call");

      $res = $call(
                   $ldap,
                   $filter_dn,
                   $f,
                   array()
                  );
      if (ldap_count_entries($ldap, $res) > 0)
      {
        authLdapDebug("authValidateUserCallback: Found entry with filter");
        return true;
      }
      authLdapDebug("authValidateUserCallback: No entry found with filter");
    }
  }
  else
  {
    authLdapDebug("authValidateUserCallback: Bind to '$dn' failed: ".ldap_error($ldap));
  }

  if ($ldap_unbind_between_attempts)
  {
    ldap_unbind($ldap);
  }

  // return failure if no connection is established
  return false;
}


/* authLdapCheckAdminGroupCallback(&$ldap, $base_dn, $dn, $user_search,
                            $user, &$object)
 * 
 * Checks if the specified username is in an admin group
 *
 * &$ldap       - Reference to the LDAP object
 * $base_dn     - The base DN
 * $dn          - The user's DN
 * $user_search - The LDAP filter to find the user
 * $user        - The user name
 * &$object     - Reference to the generic object
 * 
 * Returns:
 *   false    - Not in the admin group
 *   true     - In the admin group
 */
function authLdapCheckAdminGroupCallback(&$ldap, $base_dn, $dn, $user_search,
                                         $user, &$object)
{
  $admin_group_dn = $object['config']['ldap_admin_group_dn'];
  $group_member_attrib = $object['config']['ldap_group_member_attrib'];

  authLdapDebug("authLdapCheckAdminGroupCallback: base_dn '$base_dn' ".
                "dn '$dn' user_search '$user_search' user '$user'");

  if ($ldap && $base_dn && $dn && $user_search)
  {
    $res = ldap_read($ldap,
                     $dn,
                     "(objectclass=*)",
                     array(utf8_strtolower($group_member_attrib)) );
                     
    if (ldap_count_entries($ldap, $res) > 0)
    {
      authLdapDebug("authCheckAdminGroupCallback: search successful".
                    " '$group_member_attrib'");
      $entries = ldap_get_entries($ldap, $res);
      foreach ($entries[0][utf8_strtolower($group_member_attrib)] as $group)
      {
        if (strcasecmp($group, $admin_group_dn) == 0)
        {
          authLdapDebug("authCheckAdminGroupCallback: Admin group successfully found in user object");
          return true;
        }
      }
      authLdapDebug("authCheckAdminGroupCallback: Admin group not found in user object");
    }
  }
  return false;
}


/* authGetUserLevel($user)
 *
 * Determines the users access level
 *
 * $user - The user name
 *
 * Returns:
 *   The users access level
 */
function authGetUserLevel($user)
{
  global $ldap_admin_group_dn;
  
  // cache the user levels for performance
  static $user_levels = array();
  
  // User not logged in, user level '0'
  if (!isset($user))
  {
    return 0;
  }
  
  if (isset($user_levels[$user]))
  {
    return $user_levels[$user];
  }
  
  if ($ldap_admin_group_dn)
  {
    $object = array();
    $res = authLdapAction("authLdapCheckAdminGroupCallback", $user, $object);
    if ($res)
    {
      $level = 2;
    }
    else
    {
      $level = 1;
    }
  }
  elseif (authLdapIsAdmin($user))
  {
    $level = 2;
  }
  else  // Everybody else is access level '1'
  {
    $level = 1;
  }
  
  $user_levels[$user] = $level;
  
  return $user_levels[$user];
}

/* authLdapGetName($user)
 *
 * Gets the name of the user from LDAP
 *
 * $user  - The user name
 *
 * Returns:
 *   The user's name or ''
 */
function authLdapGetName($user)
{
  $name = '';
  $object = array();

  $res = authLdapAction("authLdapGetNameCallback", $user, $object);

  if ($res)
  {
    $name = $object['name'];
  }
  return $name;
}


/* authLdapGetNameCallback(&$ldap, $base_dn, $dn, $user_search,
                           $user, &$object)
 *
 * Get the name of a found user
 *
 * &$ldap       - Reference to the LDAP object
 * $base_dn     - The base DN
 * $dn          - The user's DN
 * $user_search - The LDAP filter to find the user
 * $user        - The user name
 * &$object     - Reference to the generic object
 *
 * Returns:
 *   false    - Didn't find a user
 *   true     - Found a user
 */
function authLdapGetNameCallback(&$ldap, $base_dn, $dn, $user_search,
                                 $user, &$object)
{
  $name_attrib = $object['config']['ldap_name_attrib'];

  authLdapDebug("authLdapGetNameCallback: base_dn '$base_dn' dn '$dn' ".
                "user_search '$user_search' user '$user'");

  if ($ldap && $base_dn && $dn && $user_search)
  {
    $res = ldap_read($ldap,
                     $dn,
                     "(objectclass=*)",
                     array(utf8_strtolower($name_attrib)) );
    
    if (ldap_count_entries($ldap, $res) > 0)
    {
      authLdapDebug("authLdapGetNameCallback: search successful");
      $entries = ldap_get_entries($ldap, $res);
      $object['name'] = $entries[0][utf8_strtolower($name_attrib)][0];

      authLdapDebug("authLdapGetNameCallback: name is '".
                    $object['name']."'");

      return true;
    }
  }
  return false;
}


// Check if the user is an admin
function authLdapIsAdmin($user)
{
  global $auth;
  
  if (isset($auth['admin']) && isset($user) && ($user !== ''))
  {
    foreach ($auth['admin'] as $admin)
    {
      if (strcasecmp($user, $admin) === 0)
      {
        return true;
      }
    }
  }
  
  return false;
}


function mrbs_ldap_escape ($value, $ignore='', $flags=0)
{
  if (function_exists('ldap_escape'))
  {
    return ldap_escape($value, $ignore, $flags);
  }
  else
  {
    return $value;
  }
}


/* authLdapDebug($message)
 *
 * Output LDAP debugging, if the configuration variable
 * $ldap_debug is true.
 *
 */
function authLdapDebug($message)
{
  global $ldap_debug;

  if ($ldap_debug)
  {
    error_log($message);
  }
}


// A wrapper for ldap_bind() that optionally suppresses "invalid credentials" errors.
function mrbs_ldap_bind ($link_identifier, $bind_rdn=null, $bind_password=null)
{
  global $ldap_suppress_invalid_credentials;
  
  // Suppress all errors and then look to see what the error was and then
  // trigger the error again, depending on config settings.
  $result = @ldap_bind($link_identifier, $bind_rdn, $bind_password);
  
  if (!$result)
  {
    $errno = ldap_errno($link_identifier);
    if (!$ldap_suppress_invalid_credentials || ($errno != LDAP_INVALID_CREDENTIALS))
    {
      trigger_error(ldap_err2str($errno), E_USER_WARNING);
    }
  }
 
  return $result;
}

<?php
namespace MRBS\Session;


/*
 * This is a slight variant of session_ip. 
 * Session management scheme that uses the DNS name of the computer
 * to identify users and administrators.
 * Anyone who can access the server can make bookings etc.
 *
 * To use this authentication scheme set the following
 * things in config.inc.php:
 *
 * $auth['type']    = 'none';
 * $auth['session'] = 'host';
 *
 * Then, you may configure admin users:
 *
 * $auth['admin'][] = 'DNSname1';
 * $auth['admin'][] = 'DNSname2';
 */
 
 
class SessionHost extends Session
{
  
  // No need to prompt for a name - if no DNSname is returned, ip address
  // is used
  public static function authGet()
  {
  }
  
  
  public static function getUsername()
  {
    $remotehostname = gethostbyaddr($_SERVER['REMOTE_ADDR']);
    
    return $remotehostname;
  }
}

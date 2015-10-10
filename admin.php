<?php
/**
 * @author      Julian Bogdani <jbogdani@gmail.com>
 * @copyright    BraDyUS. Communicating Cultural Heritage, http://bradypus.net 2007-2013
 * @license      MIT, See LICENSE file
 * @since      Dec 1, 2012
 */
try
{
  $root = './';
  require_once $root . 'lib/globals.inc';
  
  $admin = new admin_ctrl();
}
catch (Exception $e)
{
  error_log($e->getTraceAsString());
  
  $stop_error = $e->getMessage() ;
}
?>
<!DOCTYPE html>
<html lang="en">
  <head>
    
    <meta charset="utf-8">
    <title>bdus.CMS</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="./css/admin.css" rel="stylesheet">
    <script src="//tinymce.cachefly.net/4.2/tinymce.min.js"></script>
    <!-- HTML5 shim, for IE6-8 support of HTML5 elements -->
    <!--[if lt IE 9]>
      <script src="http://html5shim.googlecode.com/svn/trunk/html5.js"></script>
    <![endif]-->
  </head>
  <body<?php if (!$_SESSION['user_confirmed']) { echo ' class="login" '; }?>>
    
    <?php
    
    if ($stop_error)
    {
      
      if ($admin)
      {
        $admin->showError($stop_error);
      }
      else
      {
        echo '<div class="container">'
          . '<div class="alert alert-danger text-center">Something went wrong! '
            . $stop_error 
          . '</div>'
        . '</div>';
      }
    }
    else if (defined('CREATE_SITE'))
    {
      $admin->showCreateInstallForm();
    }
    else if (!$_SESSION['user_confirmed'])
    {
      $admin->showLoginForm();
    }
    else
    {
      $admin->showBody();
    }
    ?>
  
  <script src="controller.php?obj=tr&method=lang2json&param[]=true"></script>
  <script src="./js/all.min.js"></script>
  
  <?php if (!$_SESSION['user_confirmed']): ?>

  <script>
    $('#signin').on('submit', function(){
      $('#logerror').hide();
      $.post('controller.php?obj=log_ctrl&method=in', $(this).serialize(), function(data){
      if (data.status === 'success'){
          window.location.href = './admin';
          return false;
        } else {
          $('#logerror .text').html(data.text);
          $('#logerror').show();
          $('#login-button').button('reset');
        }
      }, 'json');
    });
  </script>
  <?php
  endif;
  
  if (defined('CREATE_SITE')):
  ?>
  <script>
    $('#newsite').on('submit', function(){
      $.post('./controller.php?obj=addsite_ctrl&method=build', $(this).serialize(), function(data){
        if (data.status === 'success'){
          $('#register').fadeOut();
          $('#message').fadeIn();
        } else {
          admin.message(data.text, data.status, false, true);
        }
      }, 'json');
    })
  </script>
  <?php endif; ?>
  
  </body>
</html>
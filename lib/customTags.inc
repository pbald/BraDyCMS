<?php
/**
 * @author      Julian Bogdani <jbogdani@gmail.com>
 * @copyright    BraDyUS. Communicating Cultural Heritage, http://bradypus.net 2007-2013
 * @license      MIT, See LICENSE file
 * @since      Apr 7, 2013
 */

class customTags
{


  public static function parseContent($html, Out $out, $id = false)
  {
    if (!preg_match('/\[\[(\w+)\s*([^\]]*)\]\](.*?)\[\[\/\1\]\]/s', $html))
    {
      return $html;
    }

    //return customTagParser::do_shortcode($html);
    preg_match_all('/\[\[(\w+)\s*([^\]]*)\]\](.*?)\[\[\/\1\]\]/s', $html, $customTags, PREG_SET_ORDER);

    foreach ($customTags as $customTag)
    {
      $attributes = array();
      $rawAttributes = array();
      $formatedAttributes = array();

      $originalTag = '/' . preg_quote($customTag[0], '/') . '/';

      $tag = $customTag[1];

      $rawAttributes = $customTag[2];

      $content = $customTag[3];

      preg_match_all('/([^=\s]+)="([^"]+)"/', $rawAttributes, $attributes, PREG_SET_ORDER);

      $formatedAttributes['content'] = $content;

      foreach ($attributes as $attribute)
      {
        $formatedAttributes[$attribute[1]] = $attribute[2];
      }

      $formatedAttributes['lang'] =
        ( ($_SESSION['lang'] && $_SESSION['lang'] !== cfg::get('sys_lang') ) ?
          $_SESSION['lang'] : false);

      if (method_exists('customTags', $tag))
      {
        $replace = call_user_func(array('customTags', $tag), $formatedAttributes, $out, $id);

        $html = preg_replace($originalTag, $replace, $html, 1);
      }
      else
      {
        if (method_exists($tag, 'init'))
        {
          $replace = call_user_func(array($tag, 'init'), $formatedAttributes, $out, $id);

          $html = preg_replace($originalTag, $replace, $html, 1);

          $pending[$tag] = true;
        }
      }
    }

    if (is_array($pending) && !empty($pending))
    {
      foreach ($pending as $obj => $true)
      {
        if (method_exists($obj, 'end'))
        {
          $html .= call_user_func(array($obj, 'end'));
        }
      }
    }
    return $html;
  }

  /**
   * Displays a user defined form
   * @param array $data
   *      $data[content]  is the name of the map to display (i.e. the name of the json file with configuration)
   *      $data[width],   the width of the map container element (default value: 100%)
   *      $data[height],  the height of the map container element (default value: 400px)
   * @param object $out    Instance of Out
   * @return type
   */
  public static function map($data, $out = false)
  {
    $f = new usermap_ctrl();

    return $f->showMap($data, $out);
  }

  /**
   * Displays a user defined form
   * @param array $data
   *      $data[content]  is the name of the form to display (i.e. the name of the json file with configuration)
   *      $data[nojs],    if true, no JS code will be printed
   * @param object $out    Instance of Out
   * @return type
   */
  public static function userform($data, $out = false)
  {
    $f = new userform_ctrl();

    return $f->showForm($data, $out);
  }

  /**
   *
   * @param type $data
   *  content: node name
   *  getObject: if false a well formatted html table will be returned, otherwize an array
   *  class: css class or space-separated classes to attache to main html table (getObject should be false)
   * @return string|boolean
   */
  public static function download($data)
  {
    $path2node = './sites/default/images/downloads/' . $data['content'];

    $json_file = $path2node . '/data'
      . ($_SESSION['lang'] && $_SESSION['lang'] !== cfg::get('sys_lang') ? '_' . $_SESSION['lang'] : '')
      . '.json';

    if (
      !file_exists($path2node)
      ||
      !file_exists($json_file)
    )
    {
      return false;
    }

    $fileData = json_decode(file_get_contents($json_file), true);

    if (!$fileData || !is_array($fileData))
    {
      return false;
    }

    $node_content = utils::dirContent($path2node);

    $files = array();


    foreach ($node_content as $file)
    {
      if (preg_match('/data_?([a-z]{2})?\.json/', $file) || $file == 'covers' )
      {
        continue;
      }
      $formattedName = str_replace('.', '__x__', $file);

      $cover_name = $path2node . DIRECTORY_SEPARATOR
        . 'covers' . DIRECTORY_SEPARATOR
        . pathinfo($file, PATHINFO_FILENAME) . '.jpg';
      if (file_exists($cover_name))
      {
        $cover = $cover_name;
      }
      else
      {
        $cover = false;
      }

      $files[] = array(
        'path' => $path2node . '/' . $file,
        'basename' => pathinfo($file, PATHINFO_FILENAME),
        'ext' => pathinfo($file, PATHINFO_EXTENSION),
        'title' => $fileData[$formattedName]['title'] ? $fileData[$formattedName]['title'] : $file,
        'description' => $fileData[$formattedName]['description'],
        'sort' => $fileData[$formattedName]['sort'],
        'cover' => $cover
        );
    }

    usort($files, function($a, $b){
      if ($a['sort'] === $b['sort'])
      {
        return 0;
      }

      return ($a['sort'] > $b['sort']) ? -1 : 1;
    });


    if ($data['getObject'])
    {
      return $files;
    }

    $html = '<table ' . ($data['class'] ? 'class="' . $data['class'] . '"' : '' ) . '>'
      . '<tbody>';

    foreach($files as $d)
    {
      $html .= '<tr>'
        . '<td class="cover"> ' . ($d['cover'] ? '<img src="' . $d['cover'] . '" class="img-responsive">' : '') . ' </td>'
        . '<th class="title">' . $d['title'] . '</th>'
        . '<td class="description">' . $d['description'] . '</td>'
        . '<td class="link"><a href="' . $d['path'] . '" class="downloadFile" target="_blank">' . tr::get('open_download') . '</a></td>'
        . '</tr>';
    }

    $html .= '</tbody>'
      . '</table>';

    return $html;
  }

  public static function cl_gallery($data)
  {
    return self::format_gallery($data, true);
  }

  public static function gallery($data)
  {
    return self::format_gallery($data);
  }

  private static function format_gallery($data, $responsive = false)
  {
    $gal = $data['content'];
    $class = $data['class'];
    $rel = $data['rel'] ? $data['rel'] : $gal;

    try
    {

      $gal_data = Gallery::get($gal);

      if ($responsive)
      {
        /**
         * <ul data-rel="sth" class="a b c cl_gallery">
         *   <li data-img="img_src" data-thumb="thumb_img_src">caption text</li>
         *   ...
         * </ul>
         */
        $html .= '<div class="gallery-container">'
          . '<ul class="cl_gallery ' . $gal . ($class ? ' ' . $class : '') . '" ' . 'data-rel="' . $rel . '"' . '>';

        foreach ($gal_data as $item)
        {
          $html .= '<li data-id="' . basename($item['img']) . '" data-img="' . $item['img'] . '" data-thumb="' . $item['thumb'] . '">'
            . $item['caption']
            . '</li>';
        }

        $html .='</ul>'
          . '</div>';
      }
      else
      {
        $html .= '<div class="gallery-container">'
          . '<ul class="gallery ' . $gal
          . ($class ? ' ' . $class : '') . '" data-rel="' . $rel . '">';

        foreach ($gal_data as $item)
        {
          $title = str_replace('"', '\"', strip_tags($item['caption']));
          $html .= '<li data-id="' . basename($item['img']) . '">'
            // Link will be set to external reference, if href is set or to image (fancybox) if not
            . '<a href="' . ($item['href'] ? $item['href'] : $item['img']) . '" '
                . (!$item['href'] ? 'class="fancybox" ' : '')
                . 'title="' . $title . '"  '
                . 'rel="' . $rel . '" >'
              . '<img src="' . $item['thumb'] . '" alt="' . $title . '" />'
            . '</a>'
            . '<div class="caption">'
              . ( $item['href'] ? '<a href="' . $item['href'] . '" title="' . $title . '">' . $item['caption'] . '</a>' : $item['caption'])
            . '</div>'
            . '</li>';
        }

        $html .='</ul>'
          . '</div>';
      }

      return $html;
    }
    catch (Exception $e)
    {
      error_log($e->getMessage());
      return false;
    }
  }


  /**
   *
   * @param array $data
   *  content: YouTube video ID
   *  ratio: Video ratio: 4by3 or 16by9. If defined the Twitter Bootstrap responsive layout will be used (http://getbootstrap.com/components/#responsive-embed)
   *  width: if ratio is not defined the width will be used, default value is 560
   *  height: if ratio is not defined the height will be used, default value is 315
   *  align: left, right, center, default false
   *  class: custom css class
   * @return string
   */
  public static function youTube($data)
  {
    $code = $data['content'];

    if (!$code)
    {
      return;
    }
    $width = $data['width'] ? $data['width'] : 560;
    $height = $data['height'] ? $data['height'] : 315;
    $ratio = ($data['ratio'] && ($data['ratio'] === '4by3' || $data['ratio'] === '16by9')) ? $data['ratio'] : '' ? : '';
    $align = $data['align'];
    $class = $data['class'];

    return '<div class="youtube'
        . ($class ? ' ' . $class : '')
        . ($align ? ' text-' . $align : '')
        . ($ratio ? ' embed-responsive embed-responsive-' . $ratio . '' : '')
        . '"'
        . '>'
      . '<iframe '
        . ( $ratio ? ' class="embed-responsive-item" ' : ' width="' . $width . '" height="' . $height . '" ')
        .'src="http://www.youtube.com/embed/' . $code . '?rel=0" frameborder="0" allowfullscreen></iframe>'
    . '</div>';
  }

  public static function vimeo($data)
  {
    $code = $data['content'];
    $width = $data['width'] ? $data['width'] : 560;
    $height = $data['height'] ? $data['height'] : 315;

    return '<iframe src="http://player.vimeo.com/video/' . $code . '" width="' . $width . '" height="' . $height . '" frameborder="0" webkitAllowFullScreen mozallowfullscreen allowFullScreen></iframe>';
  }


  public static function twitter($data)
  {
    $user = $data['user'];
    $widget_id = $data['id'];
    $content = $data['content'] ? $data['content'] : 'Tweets by @' . $data['user'];

    return '<a class="twitter-timeline"  href="https://twitter.com/' . $user . '"  data-widget-id="' . $widget_id . '">' . $content. '</a>' .
      '<script>!function(d,s,id){var js,fjs=d.getElementsByTagName(s)[0];if(!d.getElementById(id)){js=d.createElement(s);js.id=id;js.src="//platform.twitter.com/widgets.js";fjs.parentNode.insertBefore(js,fjs);}}(document,"script","twitter-wjs");</script>';
  }

  /**
   * Requires and initializates Facebook's SDK
   * @param string $lang Two digits language definition
   * @return sting
   */
  private static function fb_sdk($lang = false)
  {
    if (!isset($lang) || !preg_match('/([a-z]{2})_([A-Z]{2})/', $lang))
    {
      $lang = $_SESSION['lang'] ? $_SESSION['lang'] : cfg::get('sys_lang');
      $lang = strtolower($lang) . '_' . strtoupper($lang);
    }

    $html = <<<EOD
<div id="fb-root"></div>
<script>(function(d, s, id) {
  var js, fjs = d.getElementsByTagName(s)[0];
  if (d.getElementById(id)) return;
  js = d.createElement(s); js.id = id;
  js.src = "//connect.facebook.net/{$lang}/sdk.js#xfbml=1&version=v2.3";
  fjs.parentNode.insertBefore(js, fjs);
}(document, 'script', 'facebook-jssdk'));</script>
EOD;

    return $html;
  }

  /**
   * Facebook like box widget (https://developers.facebook.com/docs/plugins/comments
   * @param array $data array of parameters. All parameters are optional
   * @return string
   */
  public static function fb_comments($data)
  {
    $url = $data['content'] ? $data['content'] : 'http://' . $_SERVER["SERVER_NAME"] . $_SERVER["REQUEST_URI"];

    $html = self::fb_sdk($data['lang']) . '<div class="fb-comments" data-href="' . $url . '" ';

    foreach (array('colorscheme', 'mobile', 'num_posts', 'order_by', 'width') as $el)
    {
      if ($data[$el]){
        $html .= ' data-' . str_replace('_', '-', $el) . '="' . $data[$el] . '"';
      }
    }
    $html .= '></div>';

    return $html;

  }

  /**
   * Facebook follow button widget (https://developers.facebook.com/docs/plugins/follow-button
   * @param array $data array of parameters. All parameters, except content (or href) are optional
   * @return string
   */
  public static function fb_follow($data)
  {
    if (!$data['href'] && !$data['content'])
    {
      return false;
    }

    $html = self::fb_sdk($data['lang'])
      . '<div class="fb-follow" data-href="' . ($data['href'] ? $data['href'] : $data['content']) . '" ';

    foreach (array('colorscheme', 'kid_directed_site', 'layout', 'show_faces', 'width') as $el)
    {
      if ($data[$el]){
        $html .= ' data-' . str_replace('_', '-', $el) . '="' . $data[$el] . '"';
      }
    }
    $html .= '></div>';

    return $html;
  }

  /**
   * Facebook like box widget (https://developers.facebook.com/docs/plugins/like-box-for-pages)
   * @param array $data array of parameters. All parameters except href are optional
   * @return string
   */
  public static function fb_like_box($data)
  {
    if (!$data['href'] && !$data['content'])
    {
      return;
    }
    $html = self::fb_sdk($data['lang'])
      . '<div class="fb-like-box" ' .
        ' data-href="' . ($data['content'] ? $data['content'] : $data['href']) . '" ';
    foreach (array('colorscheme', 'force_wall', 'header', 'height', 'show_border', 'show_faces', 'stream', 'width') as $el)
    {
      if ($data[$el]){
        $html .= ' data-' . str_replace('_', '-', $el) . '="' . $data[$el] . '"';
      }
    }
    $html .= '></div>';
    return $html;
  }

  /**
   * Facebook like button widget https://developers.facebook.com/docs/plugins/like-button
   * @param array $data array of parameters. All parameters, except content (or href) are optional
   * @return string
   */
  public static function fb_like($data)
  {
    $url = $data['content'] ? $data['content'] : 'http://' . $_SERVER["SERVER_NAME"] . $_SERVER["REQUEST_URI"];

    $html = self::fb_sdk($data['lang'])
      . '<div class="fb-like" data-href="' . $url . '" ';

    foreach (array('action', 'colorscheme', 'kid_directed_site', 'layout', 'ref', 'share', 'show_faces', 'width') as $el)
    {
      if ($data[$el]){
        $html .= ' data-' . str_replace('_', '-', $el) . '="' . $data[$el] . '"';
      }
    }
    $html .= '></div>';

    return $html;
  }

  public static function fb_send($data)
  {
    return self::fb_sdk($data['lang']) . '<div class="fb-like" ' .
        ' data-href="http://' . $_SERVER["SERVER_NAME"] . $_SERVER["REQUEST_URI"] . '" ' .
        ($data['font'] ? ' data-font="' . $data['font'] . '" ' : '') .
        ($data['colorscheme'] ? ' data-colorscheme="' . $data['colorscheme'] . '" ' : '') .
        ($data['ref'] ? ' data-ref="' . $data['ref'] . '" ' : '') .
        ' ></div>';
  }

  private static function twitt_sdk()
  {
    return <<<EOD
<script>!function(d,s,id){var js,fjs=d.getElementsByTagName(s)[0],p=/^http:/.test(d.location)?'http':'https';if(!d.getElementById(id)){js=d.createElement(s);js.id=id;js.src=p+'://platform.twitter.com/widgets.js';fjs.parentNode.insertBefore(js,fjs);}}(document, 'script', 'twitter-wjs');</script>
EOD;
  }

  /**
   * Twitter share button
   * @param array $data: available option:
   *      via: Twitter username without @
   *      text: Tweet's text
   *      hashtag: Tweet's hashtag
   *      count: if 'none' no count will be shown
   *      lang: two-digits language id
   *      content: URL to share
   * @return string
   */
  public static function twitt_share($data)
  {
    if ($data['content'])
    {
      $data['url'] = $data['content'];
    }
    $html = '<a href="https://twitter.com/share" class="twitter-share-button"';

    foreach (array('via', 'text', 'hashtags', 'count', 'url', 'lang') as $el)
    {
      if ($data[$el])
      {
        $html .= ' data-' . $el . '="' . $data[$el] . '"';
      }
    }

    $html .= '>Tweet</a>' . self::twitt_sdk();

    return $html;
  }

  private static function gplus_sdk()
  {
    return <<<EOD
<script src="https://apis.google.com/js/platform.js" async defer></script>
EOD;
  }

  public static function gplus_plusone($data)
  {
    if ($data['content'])
    {
      $data['href'] = $data['content'];
    }
    $html = '<div class="g-plusone"';

    foreach (array('href', 'size', 'annotation', 'width', 'align', 'expandTo', 'callback', 'onstartinteraction', 'onendinteraction', 'recommendations', 'count') as $el)
    {
      if ($data[$el])
      {
        $html .= ' data-' . $el . '="' . $data[$el] . '"';
      }
    }
    $html .= '></div>' . self::gplus_sdk();

    return $html;
  }

  /**
   *
   * @param array $data
   *      pubid: public id string provided by addthis.
   *      content: html to show. Default: addthis small image
   *      only_js: if true no html will be showed
   * @return string
   */
  public static function addThis($data)
  {
    $pubid = $data['pubid'] ? $data['pubid'] : 'pubid=ra-4d8b051f4951c18f';

    return ( $data['only_js'] ? '' :
      ( $data['content'] ?
        '<a class="addthis_button" href="http://www.addthis.com/bookmark.php?v=300&amp;pubid=' . $pubid . '">' . $data['content'] . '</a>'
        :
        '<div class="addthis_sharing_toolbox"></div>'
      )
      )
      . '<script type="text/javascript" src="//s7.addthis.com/js/300/addthis_widget.js#pubid=' . $pubid . '" async="async"></script>';
  }

  /**
   *
   * @param array $data
   *           content*: skype username
   *           type: call, chat, dropdown. Default: call
   *           imageSize: 10, 12, 14, 16, 24, 32. Default: 32
   * @return string
   */
  public static function skype($data)
  {
    $uid = uniqid('skype');

    return '<script type="text/javascript" src="http://cdn.dev.skype.com/uri/skype-uri.js"></script>' .
      '<div id="' . $uid . '">' .
        '<script type="text/javascript">' .
          'Skype.ui({' .
            '"name": "' . ( $data['type'] ? $data['type'] : 'call' ) . '",' .
            '"element": "' . $uid . '",' .
            '"participants": ["' . $data['content'] . '"],' .
            '"imageSize": ' . ( $data['imageSize'] ? $data['imageSize'] : '32' ) . '' .
          '});' .
        '</script>' .
      '</div>';
  }

  /**
   *
   * @param array $data
   *     content*: file url
   *     width: widget's width, default 100%
   *     height: widget's height, default 166
   *     color: widget's color, default #ff6600
   *     autoplay: if true autoplay will be enabled, default false
   *     artwork: if true artwork will be shown, default false
   * @return string
   */
  public static function soundcloud($data)
  {
    return '<iframe ' .
      ' width="' . ($data['width'] ? $data['width'] : '100%') . '" ' .
      ' height="' . ($data['height'] ? $data['height'] : '166') . '" scrolling="no" ' .
      ' frameborder="no" ' .
      ' src="https://w.soundcloud.com/player/?' .
        'url=' . $data['content'] . '&amp;' .
        'color=' . ( $data['color'] ? $data['color'] : '#ff6600') . '&amp;' .
        'auto_play=' . ( ($data['autoplay'] && $data['autoplay'] != 'false' ) ?  'true' : 'false') . '&amp;' .
        'show_artwork=' . ( ($data['artwork'] && $data['artwork'] != 'false' ) ?  'true' : 'false') . '">' .
    '</iframe>';
  }

  /**
   *
   * @param array $data
   *     path*: image file relative url
   *     width:  main div (container) width
   *     content: image caption
   *    fancybox: boolean, if true the thumbnail will be displayed and the original file will be shown on click
   *     align: image alignent (left|right)
   *     href: href of image link
   *     href_class: css class of href link
   *    gal: string or false. If true the string will be used as rel attribute to create fancybox galleries
   * @param object $out    Instance of Out
   * @param int $id        Article id
   * @return string
   */
  public static function fig($data, $out = false, $id = false)
  {
    $new_data = array();
    if (!$data['path'])
    {
      return false;
    }

    $new_data['content'] = $data['path'];
    $new_data['width'] = $data['width'];
    $new_data['height'] = $data['height'];
    $new_data['caption'] = $data['content'];
    $new_data['fancybox'] = $data['fancybox'];
    $new_data['align'] = $data['align'];
    $new_data['href'] = $data['href'];
    $new_data['href_class'] = $data['href_class'];
    $new_data['gal'] = $data['gal'];

    return self::figure($new_data, $out, $id);
  }


  /**
   *
   * @param array $data
   *     content*: image file relative url
   *     width:  main div (container) width
   *     caption: image caption
   *    fancybox: boolean, if true the thumbnail will be displayed and the original file will be shown on click
   *     align: image alignent (left|right)
   *     href: href of image link
   *     href_class: css class of href link
   * @param object $out    Instance of Out
   * @param int $id        Article id
   * @return string
   */
  public static function figure($data, $out = false, $id = false)
  {
    // Look for file inside images/articles OR images/ folder
    if($id && file_exists('./sites/default/images/articles/media/' . $id . '/' . $data['content']))
    {
      $filename = './sites/default/images/articles/media/' . $id . '/' . $data['content'];
    }
    else if (file_exists('./sites/default/images/articles/' . $data['content']))
    {
      $filename = './sites/default/images/articles/' . $data['content'];
    }
    else if (file_exists('./sites/default/images/' . $data['content']))
    {
      $filename = './sites/default/images/' . $data['content'];
    }
    else if (file_exists($data['content']))
    {
      $filename = $data['content'];
    }
    else
    {
      return false;
    }

    // Format ALT text
    $alt = str_replace('"', '', strip_tags(
      $data['caption'] ? $data['caption'] : $data['content']
      ));

    // filename will the thumbanail filename if fancybox parameter is true and thumb exists
    if( $data['fancybox'])
    {
      $dest_file = $filename;

      // Get the file name (clean path data)
      $name = basename($filename);

      // Format the thumbnail path: same as file path + thumbs/
      $thumb = str_replace($name, 'thumbs/' . $name, $filename);

      if (file_exists($thumb))
      {
        $filename = $thumb;
      }
    }

    // Set figure_width if width or align attributes are provided
    if ($data['align'] || $data['width'])
    {
      if ($data['width'])
      {
        $figure_width = $data['width'];
      }
      else
      {
        $file_dim = getimagesize($filename);
        $figure_width = $file_dim[0] . 'px';
      }
    }


    $html = '<div class="figure' . ($data['align'] ? ' ' . $data['align'] : '') . '" '
      . ($figure_width ? ' style="max-width: ' . $figure_width . '"' : '') . '>' .
      '<div class="image">';

    if ($dest_file || $data['href'])
    {
      $html .= '<a '
        . ($data['gal'] && $data['gal'] !== '' ? 'rel="' . $data['gal'] . '"' : '')
        . ' href="' . ($data['href'] ? $data['href'] : $dest_file ) . '" '
        . 'class="fancybox' . ($data['href_class'] ? ' ' . $data['href_class'] .' ' : '') . '"'
        . ($data['fancybox'] && $data['href'] ? ' data-fancybox-type="iframe" ' : '')
        . ($data['caption'] ? ' title="' . $alt . '" ' : '')
        . '>';
    }

    $html .= '<img src="' . $filename . '" alt="' . $alt . '" />';

    if ($dest_file || $data['href'])
    {
      $html .= '</a>';
    }
    $html .= '</div>' .
      ($data['caption'] ?
          '<div class="caption">' . $data['caption'] . '</div>'
          :
          '') .
    '</div>';

    return $html;
  }

  /**
   *
   * @param array $data
   *       content*: prezi id
   *       width
   *       height
   *       bgcolor
   *       lock_to_path
   *       autoplay
   *       autohide_ctrls
   * @return string
   */
  public static function prezi($data)
  {
    $html = '<iframe ' .
      'src="http://prezi.com/embed/' . $data['content'] . '/?' .
      'bgcolor=' . ($data['bgcolor'] ? $data['bgcolor'] : 'ffffff') . '&amp;' .
      'lock_to_path=' . ( $data['lock_to_path'] ? $data['lock_to_path'] : '0') . '&amp;' .
      'autoplay=' . ($data['autoplay'] ? $data['autoplay'] : '0') . '&amp;' .
      'autohide_ctrls=' . ($data['autohide_ctrls'] ? $data['autohide_ctrls'] : '0') . '&amp;' .
      'features=undefined&amp;disabled_features=undefined" ' .
      'width="' . ( $data['width'] ? $data['width'] : '550') . '" ' .
      'height="' . ( $data['height'] ? $data['height'] : '550') . '" ' .
      'frameBorder="0"></iframe>';

    return $html;
  }


  /**
   * Enables Disqus comments in page. Disqus shortname should be provided as tag content
   * @param array $data
   *    content: (required) site shortname in disqus
   * @example [[disqus]]cliffyoungltd[[/disqus]]
   */
  public static function disqus($data)
  {
    if (!$data['content'])
    {
      return false;
    }

    $html = '<div id="disqus_thread"></div>' .
            '<script type="text/javascript">' .
              "var disqus_shortname = '{$data['content']}';" .
              "(function() {" .
                "var dsq = document.createElement('script'); dsq.type = 'text/javascript'; dsq.async = true;" .
                "dsq.src = '//' + disqus_shortname + '.disqus.com/embed.js';" .
                "(document.getElementsByTagName('head')[0] || document.getElementsByTagName('body')[0]).appendChild(dsq);" .
            "})();" .
          "</script>";
  return $html;
  }


  /**
   * @param type $data
   *    content: (required) path to swf file.
   *    width: object's width
   *    height: object's height
   *    other artguments will be used as paramaeters
   * @example [[flash width="200" height="100" quality="height"]]./sites/default/images/flash/my.swf[[/flash]]
   */
  public static function flash($data)
  {
    if (!$data['content'])
    {
      return false;
    }

    //http://stackoverflow.com/questions/1333202/embed-flash-in-html
    $html = '<object ' .
        'type="application/x-shockwave-flash" ' .
        'data="' . $data['content'] . '" ' .
        ($data['width'] ? ' width="' . $data['width'] . '"' : '') .
        ($data['height'] ? ' height="' . $data['height'] . '"' : '') .
        '> ' .
      '<param name="movie" value="' . $data['content'] . '" />';
    foreach ($data as $k => $v)
    {
      if ($k !== 'content' && $k !== 'width' && $k !== 'height')
      {
        $html .= '<param name="' . $k . '" value="' . $v . '" />';
      }
    }

    $html .= '<embed src="' . $data['content'] . '" quality="high" />' .
      '</object>';
    return $html;
  }

  /**
   * Returns well formatted html with link to internal article
   * @param array $data
   *    content: link's text
   *    art:   destination article's textid, or 'home' for link to home page
   *    title: title attribute, optional
   *    rel: rel attribute, optional
   *    class: class attribute, optional
   *    id: id attribute, optional
   *
   * @return string
   */
  public static function link($data)
  {
    $lang =
        ( ($_SESSION['lang'] && $_SESSION['lang'] !== cfg::get('sys_lang') ) ?
          $_SESSION['lang'] : false);

    $html = '<a '
      . ($data['title'] ? ' title="' . $data['title']. '"' : '')
      . ($data['rel'] ? ' rel="' . $data['rel']. '"' : '')
      . ($data['class'] ? ' class="' . $data['class']. '"' : '')
      . ($data['id'] ? ' id="' . $data['id']. '"' : '')
      . 'href="' . (
      link::to_article($data['art'] === 'home' ? './' : $data['art'], $lang)) .
      '">' . $data['content'] . '</a>';

    return $html;
  }


  /**
   * Outputs well formatted html string that embeds a google calendar
   * @param  array $data array of input data:
   *                     content: (string) (required) google calendar id eg: something@group.calendar.google.com
   *                     showPrint: (boolean) (optional, default false): if true the print option will be visible
   *                     showTabs: (boolean) (optional, default false): if true the tabs will be visible
   *                     showCalendars: (boolean) (optional, default false): if true the list of calendars be visible
   *                     height: (int|false) (optional, default 600): the height in pixels of the calendar
   *                     width: (int|false) (optional, default 800): the width in pixels of the calendar
   *                     wkst: (int|false) (optional, default 2): start day of the week 1: sunday, 2 monday, etc..
   *                     bgcolor: (string|false) (optional, default FFFFFF): background color code
   *                     color: (string|false) (optional, default 8C500B): Text color
   *                     ctz: (string|false) (optional, default Europe/Rome): Time zone
   *                     mode: (string|false) (optional, default false): Calndar mode, one of: WEEK or AGENDA
   *                     hl: (string|false) (optional, default system language): Language of the calendar
   * @return string       valid html
   */
  public static function gcalendar($data)
  {
    if (!$data['content'])
    {
      return false;
    }

    $opts = array(
      'showTabs='. ($data['showTabs'] ? '1' : '0'),
      'showCalendars=' . ($data['showCalendars'] ? '1' : '0'),
      'showTz=' . ($data['showTz'] ? '1' : '0'),
      'showPrint=' . ($data['showPrint'] ? '1' : '0'),
      'height='  . ($data['height'] ? $data['height'] : '600'),
      'wkst=' . ($data['wkst'] ? $data['wkst'] : '2'), // start day of the week 1: sunday, 2 monday
      'width='  . ($data['width'] ? $data['width'] : '800'),
      'bgcolor=' . ($data['bgcolor'] ? '%23' . $data['bgcolor'] : '%23FFFFFF'),
      'src=' . urlencode($data['content']),
      'color=' . ($data['showTz'] ? '#' . $data['color'] : '#8C500B'),
      'ctz=' . ($data['ctz'] ? urlencode($data['ctz']) : 'Europe%2FRome'),
      'mode=' . (($data['mode'] && in_array(strtoupper($data['mode']), array('WEEK', 'AGENDA'))) ? strtoupper($data['mode']) : false),
      'hl=' . ($data['hl'] ? $data['hl'] : $_SESSION['lang'])
      );

    $html = '<div class="google-calendar-container">'
      . '<iframe src="https://calendar.google.com/calendar/embed?'
      . implode('&amp;', $opts) . '"'
      . 'style="' . ($data['border'] ? $data['border'] : 'border-width:0') . '" '
      . 'width="' . ($data['width'] ? $data['width'] : 800) . '" '
      . 'height="' . ($data['height'] ? $data['height'] : 600) . '" '
      . 'frameborder="0" '
      . 'scrolling="no"></iframe>'
      . '</div>';

    return $html;
  }

  /**
   * Returns valid html link string to download file
   * @param  array $data array of input data
   *                     content: (string) (optional, default: false): Link text. If not available the file parameter will be used. The variable {tot}, if used, will be replaced with the total number of downloads for the file
   *                     file: (string) (required) absolute or relative path (some default system paths will be tested) to file to be downloaded.
   *                     * any other element will be automatically added as attribute to main <a> element
   * @return string       valid HTML or false
   */
  public static function dwnl($data)
  {
    $file = $data['file'];
    $text = $data['content'] ? $data['content'] : $file;
    if (!$file)
    {
      return false;
    }
    $tot    = DownloadAndCount::getCount($file);
    $text   = str_replace('{tot}', $tot, $text);

    $html = '<a ' . $title . $rel . $class;
    foreach ($data as $k => $v)
    {
      if (!in_array(strtolower($k),  ['content', 'file']))
      {
        $html .= " "  . $k . '="' . str_replace('"', '\"', $v) . '"';
      }
    }
    $html .= 'href="' . link::to_article('download'). '/?file=' . $file . '"">'
        . $text;
    $html .=  '</a>';

    return $html;
  }

}

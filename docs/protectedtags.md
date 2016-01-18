# Password protect sections of your site with passwords

BraDyCMS has a built-in (core) plugin to help you easily setup and manage one or more password protected sections of your web site.

You can easily add or edit authorized users to access specific parts of the sites and also define which parts of the site to set under protection.

## Admins
### How can I control access some parts of my website?
Protecting one or more areas of a web site is a matter of:
1. defining which articles will not be visible to generic users (whih articles am I going to protect)
2. defining a list of users who, once authenticated, are allowed to view these articles.

#### Select the content to protect
Protecting an article form being viewed by anyone, by a special username and password authentication is as easy as adding one or more tags to an article. The first step to setup one or more password protected areas is to define one or more tags, each for each protected area.

First at all decide the tag or tags to use for each protected area and [add them in the sites tag list](#tags/manage).

After this tag [all articles](#article/all) you want to protect with one or more the defined tags. **Notice** that an article can belong to one, two or more protected areas. It depends on the tags you use.

#### Select the authenticated users who can access protected content

You can easily setup a list of users and specify for each of them what tags he can access after authentication.

The plugin is located in:

    Main menu > Articles > Password protected tags
Or use [direct link](#protectedtags/users).

For each user you should enter a valid email address, a (strong) password and select one ore more tags he can access after authentication. This list is his **whitelist** of tags.

The sum of the **whitelists** of all users constitutes the main tags **blacklist**, ie. tags that unauthenticated users can not access.

---

If you have setup a list of tags, used them to tag article you want to protect, and defined a list of users who are allowed, once authenticated, to access these articles... **congrats, you just finished protecting one or more areas of your sites**.

---

## Designers
### How to setup templates to support password protected content

Designers can use three special methods of the [html object](#docs/read/tmpl_html) to easily setup one or more password protection for part or parts of the site content. These methods are:
* `html.canView`: returns `true` or `false` and tells you if the current content is protected or not (if content is in the **blacklist** and users is **notauthenticared**)
* `html.loginForm`: shows a well formatted html form to use for secure login
* `html.logoutButton`: if user is authenticated shows a button to use for logout

For a detailed description of these methods please refer to [their specific documentation](#docs/read/tmpl_html).

#### Example
    {# check if current content (tag blog or single article) is protected #}
    {% if html.canView == false %}

    {# protected content #}
    <div class="container>
      <div class="row">
        <div class="col-sm-4 col-sm-offset-4">
          {{ html.loginForm({
            'email_cont': 'form-group',
            'email_input': 'form-control',
            'password_cont': 'form-group',
            'password_input': 'form-control',
            'submit_input': 'btn btn-success'
            }) }}
        </div>
      </div>
    </div>
    {% else %}
      {# unprotected content #}
      {% if file_exists('sites/default/' ~ html.getContext ~ '.twig') %}
        {% include html.getContext ~ '.twig' %}
      {% else %}
        {% include 'not_found.twig' %}
      {% endif %}
    {% endif %}
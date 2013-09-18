Percolate Integration
=====================

A module that provides integration between the Percolate API and Drupal.
https://github.com/percolate/Drupal-Percolate


Features
========

- Admin interface for entering Percolate API, Percolate user
- PHP library for interacting with the Percolate API.
- UI for importing percolate posts and mapping them to content types
- Automated imports via cron
- Mapping Drupal users to Percolate users.


Usage
========

As a content editor / manager, you'll want to enable Percolate Integration and
Percolate Integration Import. Then, enter your API key at
admin/config/content/percolate. Map your site's users to their Percolate user
accounts at admin/config/content/percolate/users. You can find your API key and
user ID at https://percolate.com/settings/publishing. Now you're good to import,
just navigate to admin/content/percolate and fill in the import form. You can
set the form to auto-import at admin/config/content/percolate/import.

As a developer, you'll only need the pi module, pi_import is optional. This'll
give you access to the Percolate API by calling the pi_build_api function. You
can see what's available in phpercolate/Api.php.

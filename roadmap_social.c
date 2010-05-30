/* roadmap_social.c - Manages social network (Twitter / Facebook) accounts
 *
 * LICENSE:
 *
 *   Copyright 2008 Avi B.S
 *
 *   This file is part of RoadMap.
 *
 *   RoadMap is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License V2 as published by
 *   the Free Software Foundation.
 *
 *   RoadMap is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with RoadMap; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 */

#include <stdlib.h>
#include <string.h>

#include "roadmap_main.h"
#include "roadmap_config.h"
#include "roadmap_social.h"
#include "roadmap_dialog.h"
#include "roadmap_messagebox.h"
#include "ssd/ssd_dialog.h"
#include "ssd/ssd_container.h"
#include "ssd/ssd_text.h"
#include "ssd/ssd_choice.h"
#include "ssd/ssd_button.h"
#include "ssd/ssd_entry.h"
#include "ssd/ssd_checkbox.h"
#include "ssd/ssd_separator.h"
#include "ssd/ssd_bitmap.h"
#include "ssd/ssd_confirm_dialog.h"
#include "ssd/ssd_progress_msg_dialog.h"

#include "Realtime/Realtime.h"
#include "Realtime/RealtimeDefs.h"

#include "roadmap_browser.h"

static const char *yesno[2];

static SsdWidget CheckboxDestinationTwitter[ROADMAP_SOCIAL_DESTINATION_MODES_COUNT];
static SsdWidget CheckboxDestinationFacebook[ROADMAP_SOCIAL_DESTINATION_MODES_COUNT];

static wst_handle                s_websvc             = INVALID_WEBSVC_HANDLE;

extern const char* VerifyStatus( /* IN  */   const char*       pNext,
                                /* IN  */   void*             pContext,
                                /* OUT */   BOOL*             more_data_needed,
                                /* OUT */   roadmap_result*   rc);

static wst_parser data_parser[] =
{
   { "RC", VerifyStatus}
};


//   Twitter User name
RoadMapConfigDescriptor TWITTER_CFG_PRM_NAME_Var = ROADMAP_CONFIG_ITEM(
      TWITTER_CONFIG_TAB,
      SOCIAL_CFG_PRM_NAME_Name);

//   Twitter Password
RoadMapConfigDescriptor TWITTER_CFG_PRM_PASSWORD_Var = ROADMAP_CONFIG_ITEM(
      TWITTER_CONFIG_TAB,
      SOCIAL_CFG_PRM_PASSWORD_Name);

//   Enable / Disable Tweeting road reports
RoadMapConfigDescriptor TWITTER_CFG_PRM_SEND_REPORTS_Var = ROADMAP_CONFIG_ITEM(
      TWITTER_CONFIG_TAB,
      SOCIAL_CFG_PRM_SEND_REPORTS_Name);

//   Enable / Disable Tweeting destination
RoadMapConfigDescriptor TWITTER_CFG_PRM_SEND_DESTINATION_Var = ROADMAP_CONFIG_ITEM(
      TWITTER_CONFIG_TAB,
      SOCIAL_CFG_PRM_SEND_DESTINATION_Name);

//   Enable / Disable Road goodie munching destination
RoadMapConfigDescriptor TWITTER_CFG_PRM_SEND_MUNCHING_Var = ROADMAP_CONFIG_ITEM(
      TWITTER_CONFIG_TAB,
      SOCIAL_CFG_PRM_SEND_MUNCHING_Name);
RoadMapConfigDescriptor TWITTER_CFG_PRM_SHOW_MUNCHING_Var = ROADMAP_CONFIG_ITEM(
      TWITTER_CONFIG_TAB,
      SOCIAL_CFG_PRM_SHOW_MUNCHING_Name);

//    Logged in status
RoadMapConfigDescriptor TWITTER_CFG_PRM_LOGGED_IN_Var = ROADMAP_CONFIG_ITEM(
      TWITTER_CONFIG_TAB,
      SOCIAL_CFG_PRM_LOGGED_IN_Name);


//   Enable / Disable Facebook road reports
RoadMapConfigDescriptor FACEBOOK_CFG_PRM_SEND_REPORTS_Var = ROADMAP_CONFIG_ITEM(
      FACEBOOK_CONFIG_TAB,
      SOCIAL_CFG_PRM_SEND_REPORTS_Name);

//   Enable / Disable Facebook destination
RoadMapConfigDescriptor FACEBOOK_CFG_PRM_SEND_DESTINATION_Var = ROADMAP_CONFIG_ITEM(
      FACEBOOK_CONFIG_TAB,
      SOCIAL_CFG_PRM_SEND_DESTINATION_Name);

//   Enable / Disable Facebook Road goodie munching destination
RoadMapConfigDescriptor FACEBOOK_CFG_PRM_SEND_MUNCHING_Var = ROADMAP_CONFIG_ITEM(
      FACEBOOK_CONFIG_TAB,
      SOCIAL_CFG_PRM_SEND_MUNCHING_Name);
RoadMapConfigDescriptor FACEBOOK_CFG_PRM_SHOW_MUNCHING_Var = ROADMAP_CONFIG_ITEM(
      FACEBOOK_CONFIG_TAB,
      SOCIAL_CFG_PRM_SHOW_MUNCHING_Name);

//    Logged in status - Facebook
RoadMapConfigDescriptor FACEBOOK_CFG_PRM_LOGGED_IN_Var = ROADMAP_CONFIG_ITEM(
      FACEBOOK_CONFIG_TAB,
      SOCIAL_CFG_PRM_LOGGED_IN_Name);

//    URL - Facebook
RoadMapConfigDescriptor FACEBOOK_CFG_PRM_URL_Var = ROADMAP_CONFIG_ITEM(
      FACEBOOK_CONFIG_TAB,
      FACEBOOK_CFG_PRM_URL_Name);



enum {
   DLG_TYPE_TWITTER,
   DLG_TYPE_FACEBOOK
};


/////////////////////////////////////////////////////////////////////////////////////
BOOL roadmap_social_initialize(void) {
   
   // Name
   roadmap_config_declare(SOCIAL_CONFIG_TYPE, &TWITTER_CFG_PRM_NAME_Var,
         SOCIAL_CFG_PRM_NAME_Default, NULL);

   // Password
   roadmap_config_declare_password(SOCIAL_CONFIG_TYPE,
         &TWITTER_CFG_PRM_PASSWORD_Var, SOCIAL_CFG_PRM_PASSWORD_Default);

   // Road reports
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_TYPE,
         &TWITTER_CFG_PRM_SEND_REPORTS_Var, NULL,
         SOCIAL_CFG_PRM_SEND_REPORTS_Disabled, SOCIAL_CFG_PRM_SEND_REPORTS_Enabled, NULL);

   // Destination
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_TYPE,
         &TWITTER_CFG_PRM_SEND_DESTINATION_Var, NULL,
         SOCIAL_CFG_PRM_SEND_DESTINATION_Disabled, SOCIAL_CFG_PRM_SEND_DESTINATION_City,
         SOCIAL_CFG_PRM_SEND_DESTINATION_Street, SOCIAL_CFG_PRM_SEND_DESTINATION_Full, NULL);

   // Road goodies munching
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_TYPE,
         &TWITTER_CFG_PRM_SEND_MUNCHING_Var, NULL,
         SOCIAL_CFG_PRM_SEND_MUNCHING_Disabled, SOCIAL_CFG_PRM_SEND_MUNCHING_Enabled, NULL);
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_PREF_TYPE,
            &TWITTER_CFG_PRM_SHOW_MUNCHING_Var, NULL,
            SOCIAL_CFG_PRM_SHOW_MUNCHING_No, SOCIAL_CFG_PRM_SHOW_MUNCHING_Yes, NULL);

   // Logged in status
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_TYPE,
            &TWITTER_CFG_PRM_LOGGED_IN_Var, NULL,
            SOCIAL_CFG_PRM_LOGGED_IN_No, SOCIAL_CFG_PRM_LOGGED_IN_Yes, NULL);

   // Road reports - Facebook
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_TYPE,
         &FACEBOOK_CFG_PRM_SEND_REPORTS_Var, NULL,
         SOCIAL_CFG_PRM_SEND_REPORTS_Disabled, SOCIAL_CFG_PRM_SEND_REPORTS_Enabled, NULL);

   // Destination - Facebook
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_TYPE,
         &FACEBOOK_CFG_PRM_SEND_DESTINATION_Var, NULL,
         SOCIAL_CFG_PRM_SEND_DESTINATION_Disabled, SOCIAL_CFG_PRM_SEND_DESTINATION_City,
         SOCIAL_CFG_PRM_SEND_DESTINATION_Street, SOCIAL_CFG_PRM_SEND_DESTINATION_Full, NULL);

   // Road goodies munching - Facebook
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_TYPE,
         &FACEBOOK_CFG_PRM_SEND_MUNCHING_Var, NULL,
         SOCIAL_CFG_PRM_SEND_MUNCHING_Disabled, SOCIAL_CFG_PRM_SEND_MUNCHING_Enabled, NULL);
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_PREF_TYPE,
            &FACEBOOK_CFG_PRM_SHOW_MUNCHING_Var, NULL,
            SOCIAL_CFG_PRM_SHOW_MUNCHING_No, SOCIAL_CFG_PRM_SHOW_MUNCHING_Yes, NULL);

   // Logged in status - Facebook
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_TYPE,
            &FACEBOOK_CFG_PRM_LOGGED_IN_Var, NULL,
            SOCIAL_CFG_PRM_LOGGED_IN_No, SOCIAL_CFG_PRM_LOGGED_IN_Yes, NULL);

   // URL - Facebook
   roadmap_config_declare_enumeration(SOCIAL_CONFIG_PREF_TYPE,
            &FACEBOOK_CFG_PRM_URL_Var, NULL,
            FACEBOOK_CFG_PRM_URL_Default, NULL);

   
   
   yesno[0] = "Yes";
   yesno[1] = "No";
   
   
   return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////
void roadmap_twitter_set_logged_in (BOOL is_logged_in) {
   if (is_logged_in)
      roadmap_config_set(&TWITTER_CFG_PRM_LOGGED_IN_Var, SOCIAL_CFG_PRM_LOGGED_IN_Yes);
   else
      roadmap_config_set(&TWITTER_CFG_PRM_LOGGED_IN_Var, SOCIAL_CFG_PRM_LOGGED_IN_No);
   roadmap_config_save(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////
static BOOL roadmap_social_logged_in(RoadMapConfigDescriptor *config) {
   if (0 == strcmp(roadmap_config_get(config),
                   SOCIAL_CFG_PRM_LOGGED_IN_Yes))
      return TRUE;
   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////
BOOL roadmap_twitter_logged_in(void) {
   return roadmap_social_logged_in(&TWITTER_CFG_PRM_LOGGED_IN_Var);
}

/////////////////////////////////////////////////////////////////////////////////////
BOOL roadmap_facebook_logged_in(void) {
   return roadmap_social_logged_in(&FACEBOOK_CFG_PRM_LOGGED_IN_Var);
}

static const char *roadmap_facebook_url (void) {
   return roadmap_config_get(&FACEBOOK_CFG_PRM_URL_Var);
}

/////////////////////////////////////////////////////////////////////////////////////
static void on_check_login_completed( void* ctx, roadmap_result res) {
   if (res == succeeded)
      roadmap_config_set(&FACEBOOK_CFG_PRM_LOGGED_IN_Var, SOCIAL_CFG_PRM_LOGGED_IN_Yes);
   else
      roadmap_config_set(&FACEBOOK_CFG_PRM_LOGGED_IN_Var, SOCIAL_CFG_PRM_LOGGED_IN_No);
   roadmap_config_save(TRUE);
   
   roadmap_facebook_refresh_connection();
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_check_login(void) {
   char url[256];
   char query[256];

   roadmap_log (ROADMAP_DEBUG, "check login");
   
   
   snprintf(url, sizeof(url), "%s%s", roadmap_facebook_url(), FACEBOOK_IS_CONNECTED_SUFFIX);
   snprintf(query, sizeof(query), "sessionid=%d&cookie=%s",
            Realtime_GetServerId(),
            Realtime_GetServerCookie());

   if (INVALID_WEBSVC_HANDLE != s_websvc)
      wst_term (s_websvc);
   
   s_websvc = wst_init( url, "application/x-www-form-urlencoded; charset=utf-8");
   
   wst_start_trans( s_websvc,
                   "external_facebook",
                   data_parser,
                   sizeof(data_parser)/sizeof(wst_parser),
                   on_check_login_completed,
                   (void *)s_websvc, //TODO: consider non global
                   query);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_connect(void) {
   char url[256];
   
   snprintf(url, sizeof(url), "%s%s?sessionid=%d&cookie=%s", roadmap_facebook_url(), FACEBOOK_CONNECT_SUFFIX,
            Realtime_GetServerId(),
            Realtime_GetServerCookie());
   roadmap_browser_show("Connect to Facebook", url, roadmap_facebook_check_login);
}

static void on_disconnect_completed( void* ctx, roadmap_result res) {
   ssd_progress_msg_dialog_hide();
   
   roadmap_facebook_check_login();
}

/////////////////////////////////////////////////////////////////////////////////////
static void facebook_disconnect_confirmed_cb(int exit_code, void *context){
   char url[256];
   char query[256];
   
   if (exit_code != dec_yes)
      return;
   
   roadmap_log (ROADMAP_DEBUG, "Facebook logout");
   ssd_progress_msg_dialog_show("Disconnecting Facebook...");
   
   snprintf(url, sizeof(url), "%s%s", roadmap_facebook_url(), FACEBOOK_DISCONNECT_SUFFIX);
   snprintf(query, sizeof(query), "sessionid=%d&cookie=%s",
            Realtime_GetServerId(),
            Realtime_GetServerCookie());
   
   if (INVALID_WEBSVC_HANDLE != s_websvc)
      wst_term (s_websvc);
   
   s_websvc = wst_init( url, "application/x-www-form-urlencoded; charset=utf-8");
   
   wst_start_trans( s_websvc,
                   "external_facebook",
                   data_parser,
                   sizeof(data_parser)/sizeof(wst_parser),
                   on_disconnect_completed,
                   (void *)s_websvc, //TODO: consider non global
                   query);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_disconnect(void) {
   ssd_confirm_dialog ("", "Disconnect from Facebook?", TRUE, facebook_disconnect_confirmed_cb , NULL);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_invite(void) {
   char url[256];
   
   snprintf(url, sizeof(url), "%s%s?sessionid=%d&cookie=%s", roadmap_facebook_url(), FACEBOOK_SHARE_SUFFIX,
            Realtime_GetServerId(),
            Realtime_GetServerCookie());
   roadmap_browser_show("Invite friends", url, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_twitter_login_failed(int status) {
   if (roadmap_twitter_logged_in()) {
      if (status == 701)
         roadmap_messagebox("Oops", "Updating your twitter account details failed. Please ensure you entered the correct user name and password");
      else
         roadmap_messagebox("Oops", "Twitter error");
      roadmap_twitter_set_logged_in (FALSE);
   }
   roadmap_log (ROADMAP_DEBUG, "Twitter status error (%d)", status);
}

/////////////////////////////////////////////////////////////////////////////////////
const char *roadmap_twitter_get_username(void) {
   return roadmap_config_get(&TWITTER_CFG_PRM_NAME_Var);
}

/////////////////////////////////////////////////////////////////////////////////////
const char *roadmap_twitter_get_password(void) {
   return roadmap_config_get(&TWITTER_CFG_PRM_PASSWORD_Var);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_twitter_set_username(const char *user_name) {
   roadmap_config_set(&TWITTER_CFG_PRM_NAME_Var, user_name); //AR: we save only username
   roadmap_config_save(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_twitter_set_password(const char *password) {
   roadmap_config_set(&TWITTER_CFG_PRM_PASSWORD_Var, SOCIAL_CFG_PRM_PASSWORD_Default/*password*/); //AR: we don't want to save login details
   roadmap_config_save(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////
static BOOL roadmap_social_is_sending_enabled(RoadMapConfigDescriptor *config) {
   if (0 == strcmp(roadmap_config_get(config),
                   SOCIAL_CFG_PRM_SEND_REPORTS_Enabled))
      return TRUE;
   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////
BOOL roadmap_twitter_is_sending_enabled(void) {
   return roadmap_social_is_sending_enabled(&TWITTER_CFG_PRM_SEND_REPORTS_Var);
}

/////////////////////////////////////////////////////////////////////////////////////
BOOL roadmap_facebook_is_sending_enabled(void) {
   return roadmap_social_is_sending_enabled(&FACEBOOK_CFG_PRM_SEND_REPORTS_Var);
}

/////////////////////////////////////////////////////////////////////////////////////
static void roadmap_social_set_sending(RoadMapConfigDescriptor *config, BOOL enabled) {
   if (enabled)
       roadmap_config_set(config, SOCIAL_CFG_PRM_SEND_REPORTS_Enabled);
   else
      roadmap_config_set(config, SOCIAL_CFG_PRM_SEND_REPORTS_Disabled);
   
   roadmap_config_save(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_twitter_enable_sending() {
   roadmap_social_set_sending(&TWITTER_CFG_PRM_SEND_REPORTS_Var, TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_enable_sending() {
   roadmap_social_set_sending(&FACEBOOK_CFG_PRM_SEND_REPORTS_Var, TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_twitter_disable_sending() {
   roadmap_social_set_sending(&TWITTER_CFG_PRM_SEND_REPORTS_Var, FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_disable_sending() {
   roadmap_social_set_sending(&FACEBOOK_CFG_PRM_SEND_REPORTS_Var, FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////
static int roadmap_social_destination_mode(RoadMapConfigDescriptor *config) {
   if (0 == strcmp(roadmap_config_get(config),
                   SOCIAL_CFG_PRM_SEND_DESTINATION_Full))
      return ROADMAP_SOCIAL_DESTINATION_MODE_FULL;
   else if (0 == strcmp(roadmap_config_get(config),
                        SOCIAL_CFG_PRM_SEND_DESTINATION_City))
      return ROADMAP_SOCIAL_DESTINATION_MODE_CITY;
   else
      return ROADMAP_SOCIAL_DESTINATION_MODE_DISABLED;
}

/////////////////////////////////////////////////////////////////////////////////////
int roadmap_twitter_destination_mode(void) {
   return roadmap_social_destination_mode(&TWITTER_CFG_PRM_SEND_DESTINATION_Var);
}

/////////////////////////////////////////////////////////////////////////////////////
int roadmap_facebook_destination_mode(void) {
   return roadmap_social_destination_mode(&FACEBOOK_CFG_PRM_SEND_DESTINATION_Var);
}

/////////////////////////////////////////////////////////////////////////////////////
static void roadmap_social_set_destination_mode(RoadMapConfigDescriptor *config, int mode) {
   switch (mode) {
      case ROADMAP_SOCIAL_DESTINATION_MODE_FULL:
         roadmap_config_set(config,
                            SOCIAL_CFG_PRM_SEND_DESTINATION_Full);
         break;
      case ROADMAP_SOCIAL_DESTINATION_MODE_CITY:
         roadmap_config_set(config,
                            SOCIAL_CFG_PRM_SEND_DESTINATION_City);
         break;
      default:
         roadmap_config_set(config,
                            SOCIAL_CFG_PRM_SEND_DESTINATION_Disabled);
         break;
   }
   
   roadmap_config_save(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_twitter_set_destination_mode(int mode) {
   roadmap_social_set_destination_mode(&TWITTER_CFG_PRM_SEND_DESTINATION_Var, mode);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_set_destination_mode(int mode) {
   roadmap_social_set_destination_mode(&FACEBOOK_CFG_PRM_SEND_DESTINATION_Var, mode);
}

/////////////////////////////////////////////////////////////////////////////////////
static BOOL roadmap_social_is_munching_enabled(RoadMapConfigDescriptor *config) {
   if (0 == strcmp(roadmap_config_get(config),
                   SOCIAL_CFG_PRM_SEND_MUNCHING_Enabled))
      return TRUE;
   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////
BOOL roadmap_twitter_is_munching_enabled(void) {
   return roadmap_social_is_munching_enabled(&TWITTER_CFG_PRM_SEND_MUNCHING_Var);
}

/////////////////////////////////////////////////////////////////////////////////////
BOOL roadmap_facebook_is_munching_enabled(void) {
   return roadmap_social_is_munching_enabled(&FACEBOOK_CFG_PRM_SEND_MUNCHING_Var);
}

static void roadmap_social_set_munching(RoadMapConfigDescriptor *config, BOOL enabled) {
   if (enabled)
      roadmap_config_set(config, SOCIAL_CFG_PRM_SEND_MUNCHING_Enabled);
   else
      roadmap_config_set(config, SOCIAL_CFG_PRM_SEND_MUNCHING_Disabled);
   
   roadmap_config_save(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_twitter_enable_munching() {
   roadmap_social_set_munching(&TWITTER_CFG_PRM_SEND_MUNCHING_Var, TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_enable_munching() {
   roadmap_social_set_munching(&FACEBOOK_CFG_PRM_SEND_MUNCHING_Var, TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_twitter_disable_munching() {
   roadmap_social_set_munching(&TWITTER_CFG_PRM_SEND_MUNCHING_Var, FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_disable_munching() {
   roadmap_social_set_munching(&FACEBOOK_CFG_PRM_SEND_MUNCHING_Var, FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////
BOOL roadmap_twitter_is_show_munching(void) {
   if (0 == strcmp(roadmap_config_get(&TWITTER_CFG_PRM_SHOW_MUNCHING_Var),
         SOCIAL_CFG_PRM_SHOW_MUNCHING_Yes))
      return TRUE;
   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////
static void twitter_un_empty(void){
   roadmap_main_remove_periodic (twitter_un_empty);
   //roadmap_twitter_setting_dialog();
   roadmap_messagebox("Error", "Twitter user name is empty. You are not logged in");
}

static void twitter_pw_empty(void){
   roadmap_main_remove_periodic (twitter_pw_empty);
   //roadmap_twitter_setting_dialog();
   roadmap_messagebox("Error", "Twitter password is empty. You are not logged in");
}

/////////////////////////////////////////////////////////////////////////////////////
static void twitter_network_error(void){
   roadmap_main_remove_periodic (twitter_network_error);
   roadmap_messagebox("Oops", roadmap_lang_get("There is no network connection.Updating your twitter account details failed."));
   roadmap_twitter_set_logged_in (FALSE);
   roadmap_twitter_setting_dialog();
}
/////////////////////////////////////////////////////////////////////////////////////
static int on_ok(void *context) {
   BOOL success;
   BOOL send_reports = FALSE;
   int  destination_mode = 0;
   int i;
   BOOL send_munch = FALSE;
   const char *selected;
   BOOL logged_in;
   const char *user_name;
   const char *password;
   int dlg_type = (int)context;


   if (dlg_type == DLG_TYPE_TWITTER) {
      user_name = ssd_dialog_get_value("TwitterUserName");
      password = ssd_dialog_get_value("TwitterPassword");

      logged_in = roadmap_twitter_logged_in();
   } else {
      logged_in = roadmap_facebook_logged_in();
   }

   if (!strcasecmp((const char*) ssd_dialog_get_data("TwitterSendTwitts"),
         yesno[0]))
      send_reports = TRUE;

   for (i = 0; i < ROADMAP_SOCIAL_DESTINATION_MODES_COUNT; i++) {
      if (dlg_type == DLG_TYPE_TWITTER)
         selected = ssd_dialog_get_data (CheckboxDestinationTwitter[i]->name);
      else
         selected = ssd_dialog_get_data (CheckboxDestinationFacebook[i]->name);
      if (!strcmp (selected, "yes")) {
         break;
      }
   }
   if (i == ROADMAP_SOCIAL_DESTINATION_MODES_COUNT)
      i = 0;
   destination_mode = i;

   if (roadmap_twitter_is_show_munching())
      if (!strcasecmp((const char*) ssd_dialog_get_data("TwitterSendMunch"), yesno[0]))
         send_munch = TRUE;

   if (dlg_type == DLG_TYPE_TWITTER) {
      if (user_name[0] != 0 && password[0] != 0) {
         roadmap_twitter_set_username(user_name);
         roadmap_twitter_set_password(password);
         success = Realtime_TwitterConnect(user_name, password);
         if (!success) {
            roadmap_main_set_periodic (100, twitter_network_error);
            return 1;
         }
         roadmap_twitter_set_logged_in (TRUE);
      } else if (!logged_in && (send_reports || destination_mode > 0 || send_munch)){
         if (user_name[0] == 0) {
            roadmap_main_set_periodic (100, twitter_un_empty);
            return 1;
         }

         if (password[0] == 0) {
            roadmap_main_set_periodic (100, twitter_pw_empty);
            return 1;
         }
      }

      if (send_reports)
         roadmap_twitter_enable_sending();
      else
         roadmap_twitter_disable_sending();

      roadmap_twitter_set_destination_mode(destination_mode);

      if (send_munch)
         roadmap_twitter_enable_munching();
      else
         roadmap_twitter_disable_munching();
   } else { // Facebook
      if (send_reports)
         roadmap_facebook_enable_sending();
      else
         roadmap_facebook_disable_sending();

      roadmap_facebook_set_destination_mode(destination_mode);

      if (send_munch)
         roadmap_facebook_enable_munching();
      else
         roadmap_facebook_disable_munching();
   }


   return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
static void on_dlg_close(int exit_code, void* context) {
   if (exit_code == dec_ok) {
      on_ok(context);
   }
}


#ifndef TOUCH_SCREEN
/////////////////////////////////////////////////////////////////////////////////////
static int on_ok_softkey(SsdWidget widget, const char *new_value, void *context) {
   ssd_dialog_hide_current(dec_ok);
   return 0;
}
#endif
/////////////////////////////////////////////////////////////////////
static SsdWidget space(int height) {
   SsdWidget space;
   space = ssd_container_new("spacer", NULL, SSD_MAX_SIZE, height,
         SSD_WIDGET_SPACE | SSD_END_ROW);
   ssd_widget_set_color(space, NULL, NULL);
   return space;
}

/////////////////////////////////////////////////////////////////////////////////////
int dest_checkbox_callback_twitter (SsdWidget widget, const char *new_value) {
   int i;
   for (i = 0; i < ROADMAP_SOCIAL_DESTINATION_MODES_COUNT; i++) {
      if (CheckboxDestinationTwitter[i] &&
            strcmp (widget->parent->name, CheckboxDestinationTwitter[i]->name))
         CheckboxDestinationTwitter[i]->set_data (CheckboxDestinationTwitter[i], "no");
      else
         CheckboxDestinationTwitter[i]->set_data (CheckboxDestinationTwitter[i], "yes");
   }
   return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
int dest_checkbox_callback_facebook (SsdWidget widget, const char *new_value) {
   int i;
   for (i = 0; i < ROADMAP_SOCIAL_DESTINATION_MODES_COUNT; i++) {
      if (CheckboxDestinationFacebook[i] &&
            strcmp (widget->parent->name, CheckboxDestinationFacebook[i]->name))
         CheckboxDestinationFacebook[i]->set_data (CheckboxDestinationFacebook[i], "no");
      else
         CheckboxDestinationFacebook[i]->set_data (CheckboxDestinationFacebook[i], "yes");
   }
   return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
static void create_dialog(int dlg_type) {

   SsdWidget dialog;
   SsdWidget group;
   SsdWidget box;
   SsdWidget destination;
   SsdWidget hor_space;
   SsdWidget text;
   SsdWidget bitmap;
   SsdWidget *checkbox;
   SsdCallback dest_checkbox_cb;
   int width;
   int tab_flag = SSD_WS_TABSTOP;
   BOOL checked = FALSE;
   const char * notesColor = "#3b3838"; // some sort of gray
   BOOL isTwitter = (dlg_type == DLG_TYPE_TWITTER);
   width = roadmap_canvas_width()/2;
   
   if (isTwitter) {
      dialog = ssd_dialog_new(TWITTER_DIALOG_NAME,
         roadmap_lang_get(TWITTER_TITTLE), on_dlg_close, SSD_CONTAINER_TITLE);
      checkbox = CheckboxDestinationTwitter;
      dest_checkbox_cb = dest_checkbox_callback_twitter;
   } else {
      dialog = ssd_dialog_new(FACEBOOK_DIALOG_NAME,
         roadmap_lang_get(FACEBOOK_TITTLE), on_dlg_close, SSD_CONTAINER_TITLE);
      checkbox = CheckboxDestinationFacebook;
      dest_checkbox_cb = dest_checkbox_callback_facebook;
   }

#ifdef TOUCH_SCREEN
   ssd_widget_add (dialog, space(5));
#endif

   box = ssd_container_new("UN/PW group", NULL, SSD_MAX_SIZE, SSD_MIN_SIZE,
         SSD_WIDGET_SPACE | SSD_END_ROW | SSD_ROUNDED_CORNERS
               | SSD_ROUNDED_WHITE | SSD_POINTER_NONE | SSD_CONTAINER_BORDER);

   //Accound details header
   group = ssd_container_new("Twitter Account Header group", NULL, SSD_MAX_SIZE,SSD_MIN_SIZE,
         SSD_WIDGET_SPACE | SSD_END_ROW);
   ssd_widget_set_color(group, "#000000", "#ffffff");
   ssd_widget_add(group, ssd_text_new("Label", roadmap_lang_get("Account details"),
         16, SSD_TEXT_LABEL | SSD_ALIGN_VCENTER));
   if (isTwitter)
      bitmap = ssd_bitmap_new ("twitter_icon", "Tweeter-logo",SSD_ALIGN_RIGHT|SSD_WS_TABSTOP);
   else
      bitmap = ssd_bitmap_new ("facebook_icon", "Facebook-logo",SSD_ALIGN_RIGHT|SSD_WS_TABSTOP);
   ssd_widget_add(group, bitmap);
   ssd_widget_add (group, ssd_separator_new ("separator", SSD_ALIGN_BOTTOM));
   ssd_widget_add(box, group);
   //Accound login status
   group = ssd_container_new("Twitter Account Login group", NULL, SSD_MAX_SIZE,SSD_MIN_SIZE,
         SSD_WIDGET_SPACE | SSD_END_ROW);
   ssd_widget_set_color(group, "#000000", "#ffffff");
   ssd_widget_add(group, ssd_text_new("Login Status Label", "",
            16, SSD_TEXT_LABEL | SSD_ALIGN_VCENTER));
   ssd_widget_add (group, ssd_separator_new ("separator", SSD_ALIGN_BOTTOM));
   ssd_widget_add(box, group);
   if (isTwitter) {
      //User name
      group = ssd_container_new("Twitter Name group", NULL, SSD_MAX_SIZE,SSD_MIN_SIZE,
            SSD_WIDGET_SPACE | SSD_END_ROW);
      ssd_widget_set_color(group, "#000000", "#ffffff");
      ssd_widget_add(group, ssd_text_new("Label", roadmap_lang_get("User name"),
            -1, SSD_TEXT_LABEL | SSD_ALIGN_VCENTER));
      ssd_widget_add(group, ssd_entry_new("TwitterUserName", "", SSD_ALIGN_VCENTER
            | SSD_ALIGN_RIGHT | tab_flag, 0, width, SSD_MIN_SIZE,
            roadmap_lang_get("User name")));
      ssd_widget_add(box, group);

      //Password
      group = ssd_container_new("Twitter PW group", NULL, SSD_MAX_SIZE, 40,
            SSD_WIDGET_SPACE | SSD_END_ROW);
      ssd_widget_set_color(group, "#000000", "#ffffff");

      ssd_widget_add(group, ssd_text_new("Label", roadmap_lang_get("Password"),
            -1, SSD_TEXT_LABEL | SSD_ALIGN_VCENTER));
      ssd_widget_add(group, ssd_entry_new("TwitterPassword", "", SSD_ALIGN_VCENTER
            | SSD_ALIGN_RIGHT | tab_flag, SSD_TEXT_PASSWORD, width, SSD_MIN_SIZE,
            roadmap_lang_get("Password")));
      ssd_widget_add(box, group);
   }

   ssd_widget_add(dialog, box);


   //tweet settings header
   ssd_widget_add(dialog, space(5));
   box = ssd_container_new ("Tweeter auto settings header group", NULL, SSD_MIN_SIZE, SSD_MIN_SIZE,
            SSD_WIDGET_SPACE | SSD_END_ROW);

   if (isTwitter)
      ssd_widget_add (box, ssd_text_new ("tweeter_auto_settings_header",
            roadmap_lang_get ("Automatically tweet to my followers:"), 16, SSD_TEXT_LABEL | SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE));
   else
      ssd_widget_add (box, ssd_text_new ("tweeter_auto_settings_header",
            roadmap_lang_get ("Automatically post to Facebook:"), 16, SSD_TEXT_LABEL | SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE));
   ssd_widget_set_color (box, NULL, NULL);
   ssd_widget_add (dialog, box);
   ssd_widget_add(dialog, space(5));

   //Send Reports Yes/No
   group = ssd_container_new("Send_Reports group", NULL, SSD_MAX_SIZE, SSD_MIN_SIZE,
         SSD_START_NEW_ROW | SSD_WIDGET_SPACE | SSD_END_ROW
                              | SSD_ROUNDED_CORNERS | SSD_ROUNDED_WHITE
                              | SSD_POINTER_NONE | SSD_CONTAINER_BORDER| tab_flag);
   ssd_widget_set_color(group, "#000000", "#ffffff");

   ssd_widget_add(group, ssd_text_new("Send_reports_label", roadmap_lang_get(
         "My road reports"), -1, SSD_TEXT_LABEL
         | SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE));

   ssd_widget_add(group, ssd_checkbox_new("TwitterSendTwitts", TRUE,
         SSD_END_ROW | SSD_ALIGN_RIGHT | SSD_ALIGN_VCENTER , NULL, NULL, NULL,
         CHECKBOX_STYLE_ON_OFF));
   ssd_widget_add(dialog, group);

   //example
   box = ssd_container_new ("report_example group", NULL, SSD_MIN_SIZE, SSD_MIN_SIZE,
      SSD_WIDGET_SPACE | SSD_END_ROW);
   text = ssd_text_new ("report_example_text_cont",
      roadmap_lang_get ("e.g:  Just reported a traffic jam on Geary St. SF, CA using @waze Social GPS."),
                           14, SSD_TEXT_LABEL | SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE);
   ssd_text_set_color(text,notesColor);
   ssd_widget_add (box,text );
   ssd_widget_set_color (box, NULL, NULL);
   ssd_widget_add (dialog, box);

   //Send Destination
   ssd_widget_add(dialog, space(5));
   destination = ssd_container_new("Send_Destination group", NULL, SSD_MAX_SIZE, SSD_MIN_SIZE,
            SSD_START_NEW_ROW | SSD_WIDGET_SPACE | SSD_END_ROW
                                 | SSD_ROUNDED_CORNERS | SSD_ROUNDED_WHITE
                                 | SSD_POINTER_NONE | SSD_CONTAINER_BORDER);
   box = ssd_container_new ("Destination Heading group", NULL, SSD_MAX_SIZE,SSD_MIN_SIZE,
         SSD_WIDGET_SPACE | SSD_END_ROW);
   ssd_widget_add (box, ssd_text_new ("destination_heading_label",
            roadmap_lang_get ("My destination and ETA"), 16, SSD_TEXT_LABEL
                     | SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE | SSD_END_ROW));

   ssd_widget_add (box, ssd_separator_new ("separator", SSD_ALIGN_BOTTOM));
   ssd_widget_set_color (box, NULL, NULL);
   ssd_widget_add (destination, box);
   //Disabled
   box = ssd_container_new ("Destination disabled Group", NULL, SSD_MAX_SIZE,SSD_MIN_SIZE,
         SSD_WIDGET_SPACE | SSD_END_ROW| tab_flag);
   if (roadmap_twitter_destination_mode() == ROADMAP_SOCIAL_DESTINATION_MODE_DISABLED)
      checked = TRUE;
   else
      checked = FALSE;

   checkbox[ROADMAP_SOCIAL_DESTINATION_MODE_DISABLED] = ssd_checkbox_new (SOCIAL_CFG_PRM_SEND_DESTINATION_Disabled,
               checked, SSD_ALIGN_VCENTER, dest_checkbox_cb, NULL, NULL,
               CHECKBOX_STYLE_ROUNDED);
   ssd_widget_add (box, checkbox[ROADMAP_SOCIAL_DESTINATION_MODE_DISABLED]);

   hor_space = ssd_container_new ("spacer1", NULL, 10, 14, 0);
   ssd_widget_set_color (hor_space, NULL, NULL);
   ssd_widget_add (box, hor_space);

   ssd_widget_add (box, ssd_text_new ("Destination disabled", roadmap_lang_get (
            "Disabled"), 14, SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE
            | SSD_END_ROW));

   ssd_widget_add (box, ssd_separator_new ("separator", SSD_ALIGN_BOTTOM));
   ssd_widget_set_color (box, NULL, NULL);
   ssd_widget_add (destination, box);


   // State and City
   box = ssd_container_new ("Destination city Group", NULL, SSD_MAX_SIZE,SSD_MIN_SIZE,
         SSD_WIDGET_SPACE | SSD_END_ROW| tab_flag);
   if (roadmap_twitter_destination_mode() == ROADMAP_SOCIAL_DESTINATION_MODE_CITY)
      checked = TRUE;
   else
      checked = FALSE;

   checkbox[ROADMAP_SOCIAL_DESTINATION_MODE_CITY] = ssd_checkbox_new (SOCIAL_CFG_PRM_SEND_DESTINATION_City,
               checked, SSD_ALIGN_VCENTER, dest_checkbox_cb, NULL, NULL,
               CHECKBOX_STYLE_ROUNDED);
   ssd_widget_add (box, checkbox[ROADMAP_SOCIAL_DESTINATION_MODE_CITY]);

   hor_space = ssd_container_new ("spacer1", NULL, 10, 14, 0);
   ssd_widget_set_color (hor_space, NULL, NULL);
   ssd_widget_add (box, hor_space);

   ssd_widget_add (box, ssd_text_new ("Destination city", roadmap_lang_get (
            "City & state only"), 14, SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE
            | SSD_END_ROW));

   ssd_widget_add (box, ssd_separator_new ("separator", SSD_ALIGN_BOTTOM));
   ssd_widget_set_color (box, NULL, NULL);
   ssd_widget_add (destination, box);

   // Street, State and City (future option)
   box = ssd_container_new ("Destination street Group", NULL, SSD_MAX_SIZE,SSD_MIN_SIZE,
         SSD_WIDGET_SPACE | SSD_END_ROW| tab_flag);
   if (roadmap_twitter_destination_mode() == ROADMAP_SOCIAL_DESTINATION_MODE_STREET)
      checked = TRUE;
   else
      checked = FALSE;

   checkbox[ROADMAP_SOCIAL_DESTINATION_MODE_STREET] = ssd_checkbox_new (SOCIAL_CFG_PRM_SEND_DESTINATION_Street,
               checked, SSD_ALIGN_VCENTER, dest_checkbox_cb, NULL, NULL,
               CHECKBOX_STYLE_ROUNDED);

   ssd_widget_add (box, checkbox[ROADMAP_SOCIAL_DESTINATION_MODE_STREET]);

   hor_space = ssd_container_new ("spacer1", NULL, 10, 14, 0);
   ssd_widget_set_color (hor_space, NULL, NULL);
   ssd_widget_add (box, hor_space);

   ssd_widget_add (box, ssd_text_new ("Destination street", roadmap_lang_get (
            "Street, City & State"), 14, SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE
            | SSD_END_ROW));

   ssd_widget_add (box, ssd_separator_new ("separator", SSD_ALIGN_BOTTOM));
   ssd_widget_set_color (box, NULL, NULL);
   ssd_widget_add (destination, box);
   ssd_widget_hide(box);//future option


   // Full address
   box = ssd_container_new ("Destination full Group", NULL, SSD_MAX_SIZE,SSD_MIN_SIZE,
         SSD_WIDGET_SPACE | SSD_END_ROW| tab_flag);
   if (roadmap_twitter_destination_mode() == ROADMAP_SOCIAL_DESTINATION_MODE_FULL)
      checked = TRUE;
   else
      checked = FALSE;

   checkbox[ROADMAP_SOCIAL_DESTINATION_MODE_FULL] = ssd_checkbox_new (SOCIAL_CFG_PRM_SEND_DESTINATION_Full,
               checked, SSD_ALIGN_VCENTER, dest_checkbox_cb, NULL, NULL,
               CHECKBOX_STYLE_ROUNDED);
   ssd_widget_add (box, checkbox[ROADMAP_SOCIAL_DESTINATION_MODE_FULL]);

   hor_space = ssd_container_new ("spacer1", NULL, 10, 14, 0);
   ssd_widget_set_color (hor_space, NULL, NULL);
   ssd_widget_add (box, hor_space);

   ssd_widget_add (box, ssd_text_new ("Destination full", roadmap_lang_get (
            "House #, Street, City, State"), 14, SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE
            | SSD_END_ROW));

   ssd_widget_set_color (box, NULL, NULL);
   ssd_widget_add (destination, box);

   ssd_widget_add (dialog, destination);

   //example
   box = ssd_container_new ("destination_example group", NULL, SSD_MIN_SIZE, SSD_MIN_SIZE,
      SSD_WIDGET_SPACE | SSD_END_ROW);
   text = ssd_text_new ("destination_example_text_cont",
      roadmap_lang_get ("e.g:  Driving to Greary St. SF, using @waze social GPS. ETA 2:32pm."),
                           14, SSD_TEXT_LABEL | SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE);
   ssd_text_set_color(text,notesColor);
   ssd_widget_add (box,text );
   ssd_widget_set_color (box, NULL, NULL);
   ssd_widget_add (dialog, box);

   //Send road goodie munch Yes/No
   if (roadmap_twitter_is_show_munching()) {
      ssd_widget_add(dialog, space(5));
      group = ssd_container_new("Send_Munch group", NULL, SSD_MAX_SIZE, SSD_MIN_SIZE,
            SSD_START_NEW_ROW | SSD_WIDGET_SPACE | SSD_END_ROW
                                 | SSD_ROUNDED_CORNERS | SSD_ROUNDED_WHITE
                                 | SSD_POINTER_NONE | SSD_CONTAINER_BORDER| tab_flag);
      ssd_widget_set_color(group, "#000000", "#ffffff");

      ssd_widget_add(group, ssd_text_new("Send_munch_label", roadmap_lang_get(
            "My road munching"), -1, SSD_TEXT_LABEL
            | SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE));

      ssd_widget_add(group, ssd_checkbox_new("TwitterSendMunch", TRUE,
            SSD_END_ROW | SSD_ALIGN_RIGHT | SSD_ALIGN_VCENTER, NULL, NULL, NULL,
            CHECKBOX_STYLE_ON_OFF));
      ssd_widget_add(dialog, group);

      //example
      box = ssd_container_new ("munch_example group", NULL, SSD_MIN_SIZE, SSD_MIN_SIZE,
         SSD_WIDGET_SPACE | SSD_END_ROW);
      text = ssd_text_new ("munch_example_text_cont",
         roadmap_lang_get ("e.g:  Just munched a 'waze road goodie' worth 200 points on Geary St. SF driving with @waze social GPS"),
                              14, SSD_TEXT_LABEL | SSD_ALIGN_VCENTER | SSD_WIDGET_SPACE| tab_flag);
      ssd_text_set_color(text,notesColor);
      ssd_widget_add (box,text );
      ssd_widget_set_color (box, NULL, NULL);
      ssd_widget_add (dialog, box);
   }

#ifndef TOUCH_SCREEN
   ssd_widget_set_left_softkey_text ( dialog, roadmap_lang_get("Ok"));
   ssd_widget_set_left_softkey_callback ( dialog, on_ok_softkey);
#endif

}

#ifndef IPHONE_NATIVE
/////////////////////////////////////////////////////////////////////////////////////
void roadmap_twitter_setting_dialog(void) {
   const char *pVal;

   if (!ssd_dialog_activate(TWITTER_DIALOG_NAME, (void *)DLG_TYPE_TWITTER)) {
      create_dialog(DLG_TYPE_TWITTER);
      ssd_dialog_activate(TWITTER_DIALOG_NAME, (void *)DLG_TYPE_TWITTER);
   }

   if (roadmap_twitter_logged_in())
      ssd_dialog_set_value("Login Status Label", roadmap_lang_get("Status: logged in"));
   else
      ssd_dialog_set_value("Login Status Label", roadmap_lang_get("Status: not logged in"));

   ssd_dialog_set_value("TwitterUserName", roadmap_twitter_get_username());
   ssd_dialog_set_value("TwitterPassword", roadmap_twitter_get_password());

   if (roadmap_twitter_is_sending_enabled())
      pVal = yesno[0];
   else
      pVal = yesno[1];
   ssd_dialog_set_data("TwitterSendTwitts", (void *) pVal);

   if (roadmap_twitter_is_munching_enabled())
      pVal = yesno[0];
   else
      pVal = yesno[1];
   ssd_dialog_set_data("TwitterSendMunch", (void *) pVal);
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_refresh_connection (void) {
   if (strcmp(ssd_dialog_currently_active_name(), FACEBOOK_DIALOG_NAME) == 0 ) {
      if (roadmap_facebook_logged_in())
         ssd_dialog_set_value("Login Status Label", roadmap_lang_get("Status: logged in"));
      else
         ssd_dialog_set_value("Login Status Label", roadmap_lang_get("Status: not logged in"));
   }
}

/////////////////////////////////////////////////////////////////////////////////////
void roadmap_facebook_setting_dialog(void) {
   const char *pVal;

   if (!ssd_dialog_activate(FACEBOOK_DIALOG_NAME, (void *)DLG_TYPE_FACEBOOK)) {
      create_dialog(DLG_TYPE_FACEBOOK);
      ssd_dialog_activate(FACEBOOK_DIALOG_NAME, (void *)DLG_TYPE_FACEBOOK);
   }

   if (roadmap_facebook_logged_in())
      ssd_dialog_set_value("Login Status Label", roadmap_lang_get("Status: logged in"));
   else
      ssd_dialog_set_value("Login Status Label", roadmap_lang_get("Status: not logged in"));

   if (roadmap_facebook_is_sending_enabled())
      pVal = yesno[0];
   else
      pVal = yesno[1];
   ssd_dialog_set_data("TwitterSendTwitts", (void *) pVal);

   if (roadmap_facebook_is_munching_enabled())
      pVal = yesno[0];
   else
      pVal = yesno[1];
   ssd_dialog_set_data("TwitterSendMunch", (void *) pVal);
}
#endif //IPHONE_NATIVE

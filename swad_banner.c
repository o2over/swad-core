// swad_banner.c: banners

/*
    SWAD (Shared Workspace At a Distance),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2015 Antonio Ca�as Vargas

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*****************************************************************************/
/********************************* Headers ***********************************/
/*****************************************************************************/

#include <linux/stddef.h>	// For NULL
#include <stdlib.h>		// For calloc
#include <string.h>		// For string functions

#include "swad_banner.h"
#include "swad_constant.h"
#include "swad_database.h"
#include "swad_global.h"
#include "swad_parameter.h"
#include "swad_photo.h"
#include "swad_text.h"

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

extern struct Globals Gbl;

/*****************************************************************************/
/***************************** Private constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/******************************* Private types *******************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private variables *****************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private prototypes ****************************/
/*****************************************************************************/

static void Ban_PutFormToEditBanners (void);
static void Ban_GetListBanners (const char *Query);
static void Ban_ListBannersForEdition (void);
static void Ban_PutParamBanCod (long BanCod);
static void Ban_ShowOrHideBanner (bool Hide);
static void Ban_RenameBanner (Cns_ShortOrFullName_t ShortOrFullName);
static bool Ban_CheckIfBannerNameExists (const char *FieldName,const char *Name,long BanCod);
static void Ban_PutFormToCreateBanner (void);
static void Ban_PutHeadBanners (void);
static void Ban_CreateBanner (struct Banner *Ban);

/*****************************************************************************/
/*************************** List all the banners ****************************/
/*****************************************************************************/

void Ban_SeeBanners (void)
  {
   extern const char *Txt_Banners;
   unsigned NumBan;

   /***** Get list of banners *****/
   Ban_GetListBanners ("SELECT BanCod,Hidden,ShortName,FullName,Img,WWW"
	               " FROM banners"
                       " WHERE Hidden='N'"
	               " ORDER BY ShortName");

   /***** Put banner (form) to edit banners *****/
   if (Gbl.Usrs.Me.LoggedRole == Rol_SYS_ADM)
      Ban_PutFormToEditBanners ();

   /***** Table head *****/
   Lay_StartRoundFrameTable (NULL,2,Txt_Banners);

   /***** Write all the banners *****/
   for (NumBan = 0;
	NumBan < Gbl.Banners.Num;
	NumBan++)
      /* Write data of this banner */
      fprintf (Gbl.F.Out,"<tr>"
                         "<td class=\"LEFT_MIDDLE\">"
                         "<a href=\"%s\" title=\"%s\" class=\"DAT\" target=\"_blank\">"
                         "<img src=\"%s/%s/%s\""
                         " alt=\"%s\" title=\"%s\""
                         " class=\"BANNER\" />"
                         "</a>"
                         "</td>"
                         "</tr>",
               Gbl.Banners.Lst[NumBan].WWW,
               Gbl.Banners.Lst[NumBan].FullName,
               Cfg_HTTPS_URL_SWAD_PUBLIC,Cfg_FOLDER_BANNER,
               Gbl.Banners.Lst[NumBan].Img,
               Gbl.Banners.Lst[NumBan].ShortName,
               Gbl.Banners.Lst[NumBan].FullName);

   /***** Table end *****/
   Lay_EndRoundFrameTable ();

   /***** Free list of banners *****/
   Ban_FreeListBanners ();
  }

/*****************************************************************************/
/********************* Put a banner (form) to edit banners *******************/
/*****************************************************************************/

static void Ban_PutFormToEditBanners (void)
  {
   extern const char *Txt_Edit;

   /***** Link to edit banners *****/
   fprintf (Gbl.F.Out,"<div class=\"CONTEXT_MENU\">");
   Act_PutContextualLink (ActEdiBan,NULL,"edit",Txt_Edit);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/************************** Put forms to edit banners ************************/
/*****************************************************************************/

void Ban_EditBanners (void)
  {
   extern const char *Txt_There_are_no_banners;

   /***** Get list of banners *****/
   Ban_GetListBanners ("SELECT BanCod,Hidden,ShortName,FullName,Img,WWW"
	               " FROM banners ORDER BY ShortName");

   if (Gbl.Banners.Num)
      /***** Put link (form) to view banners *****/
      Lay_PutFormToView (ActSeeBan);
   else
      /***** Help message *****/
      Lay_ShowAlert (Lay_INFO,Txt_There_are_no_banners);

   /***** Put a form to create a new banner *****/
   Ban_PutFormToCreateBanner ();

   /***** Forms to edit current banners *****/
   if (Gbl.Banners.Num)
      Ban_ListBannersForEdition ();

   /***** Free list of banners *****/
   Ban_FreeListBanners ();
  }

/*****************************************************************************/
/**************************** List all the banners ***************************/
/*****************************************************************************/

static void Ban_GetListBanners (const char *Query)
  {
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRows;
   unsigned NumBan;
   struct Banner *Ban;

   if (Gbl.DB.DatabaseIsOpen)
     {
      /***** Get banners from database *****/
      NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get banners");

      if (NumRows) // Banners found...
	{
	 Gbl.Banners.Num = (unsigned) NumRows;

	 /***** Create list with banners *****/
	 if ((Gbl.Banners.Lst = (struct Banner *) calloc (NumRows,sizeof (struct Banner))) == NULL)
	     Lay_ShowErrorAndExit ("Not enough memory to store banners.");

	 /***** Get the banners *****/
	 for (NumBan = 0;
	      NumBan < Gbl.Banners.Num;
	      NumBan++)
	   {
	    Ban = &(Gbl.Banners.Lst[NumBan]);

	    /* Get next banner */
	    row = mysql_fetch_row (mysql_res);

	    /* Get banner code (row[0]) */
	    if ((Ban->BanCod = Str_ConvertStrCodToLongCod (row[0])) < 0)
	       Lay_ShowErrorAndExit ("Wrong code of banner.");

	    /* Get if banner is hidden (row[1]) */
            Ban->IsHidden = (Str_ConvertToUpperLetter (row[1][0]) == 'Y');

	    /* Get the short name of the banner (row[2]) */
	    strcpy (Ban->ShortName,row[2]);

	    /* Get the full name of the banner (row[3]) */
	    strcpy (Ban->FullName,row[3]);

	    /* Get the image of the banner (row[4]) */
	    strcpy (Ban->Img,row[4]);

	    /* Get the URL of the banner (row[5]) */
	    strcpy (Ban->WWW,row[5]);
	   }
	}
      else
	 Gbl.Banners.Num = 0;

      /***** Free structure that stores the query result *****/
      DB_FreeMySQLResult (&mysql_res);
     }
  }

/*****************************************************************************/
/*************************** Get banner full name ****************************/
/*****************************************************************************/

void Ban_GetDataOfBannerByCod (struct Banner *Ban)
  {
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRows;

   /***** Clear data *****/
   Ban->IsHidden = false;
   Ban->ShortName[0] = Ban->FullName[0] = Ban->Img[0] = Ban->WWW[0] = '\0';

   /***** Check if banner code is correct *****/
   if (Ban->BanCod > 0)
     {
      /***** Get data of a banner from database *****/
      sprintf (Query,"SELECT Hidden,ShortName,FullName,Img,WWW"
	             " FROM banners WHERE BanCod='%ld'",
               Ban->BanCod);
      NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get data of a banner");

      if (NumRows) // Banner found...
        {
         /* Get row */
         row = mysql_fetch_row (mysql_res);

         /* Get if the banner is hidden (row[0]) */
         Ban->IsHidden = (row[0][0] == 'Y');

         /* Get the short name of the banner (row[1]) */
         strcpy (Ban->ShortName,row[1]);

         /* Get the full name of the banner (row[2]) */
         strcpy (Ban->FullName,row[2]);

         /* Get the image of the banner (row[3]) */
         strcpy (Ban->Img,row[3]);

         /* Get the URL of the banner (row[4]) */
         strcpy (Ban->WWW,row[4]);
        }

      /***** Free structure that stores the query result *****/
      DB_FreeMySQLResult (&mysql_res);
     }
  }

/*****************************************************************************/
/*************************** Free list of banners ****************************/
/*****************************************************************************/

void Ban_FreeListBanners (void)
  {
   if (Gbl.Banners.Lst)
     {
      /***** Free memory used by the list of banners *****/
      free ((void *) Gbl.Banners.Lst);
      Gbl.Banners.Lst = NULL;
      Gbl.Banners.Num = 0;
     }
  }

/*****************************************************************************/
/*************************** List all the banners ****************************/
/*****************************************************************************/

static void Ban_ListBannersForEdition (void)
  {
   extern const char *Txt_Banners;
   extern const char *Txt_Show;
   extern const char *Txt_Hide;
   unsigned NumBan;
   struct Banner *Ban;

   Lay_StartRoundFrameTable (NULL,2,Txt_Banners);

   /***** Table head *****/
   Ban_PutHeadBanners ();

   /***** Write all the banners *****/
   for (NumBan = 0;
	NumBan < Gbl.Banners.Num;
	NumBan++)
     {
      Ban = &Gbl.Banners.Lst[NumBan];

      /* Put icon to remove banner */
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td class=\"BM\">");
      Act_FormStart (ActRemBan);
      Ban_PutParamBanCod (Ban->BanCod);
      Lay_PutIconRemove ();
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Put icon to hide/show banner */
      fprintf (Gbl.F.Out,"<td class=\"BM\">");
      Act_FormStart (Ban->IsHidden ? ActShoBan :
	                             ActHidBan);
      Ban_PutParamBanCod (Ban->BanCod);
      fprintf (Gbl.F.Out,"<input type=\"image\" src=\"%s/%s_on16x16.gif\""
	                 " alt=\"%s\" title=\"%s\" class=\"ICON16x16\" />",
               Gbl.Prefs.IconsURL,
               Ban->IsHidden ? "hidden" :
        	               "visible",
               Ban->IsHidden ? Txt_Show :
        	               Txt_Hide,
               Ban->IsHidden ? Txt_Show :
        	               Txt_Hide);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Banner code */
      fprintf (Gbl.F.Out,"<td class=\"%s RIGHT_MIDDLE\">"
	                 "%ld&nbsp;"
	                 "</td>",
               Ban->IsHidden ? "DAT_LIGHT" :
        	               "DAT",
               Ban->BanCod);

      /* Banner short name */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActRenBanSho);
      Ban_PutParamBanCod (Ban->BanCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"ShortName\""
	                 " size=\"12\" maxlength=\"%u\" value=\"%s\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Ban_MAX_LENGTH_SHORT_NAME,Ban->ShortName,Gbl.FormId);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Banner full name */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActRenBanFul);
      Ban_PutParamBanCod (Ban->BanCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"FullName\""
	                 " size=\"24\" maxlength=\"%u\" value=\"%s\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Ban_MAX_LENGTH_FULL_NAME,Ban->FullName,Gbl.FormId);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Banner image */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActChgBanImg);
      Ban_PutParamBanCod (Ban->BanCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"Img\""
	                 " size=\"12\" maxlength=\"%u\" value=\"%s\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Ban_MAX_LENGTH_IMAGE,Ban->Img,Gbl.FormId);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Banner WWW */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActChgBanWWW);
      Ban_PutParamBanCod (Ban->BanCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"WWW\""
	                 " size=\"24\" maxlength=\"%u\" value=\"%s\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Cns_MAX_LENGTH_WWW,Ban->WWW,Gbl.FormId);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>"
                         "</tr>");
     }

   Lay_EndRoundFrameTable ();
  }

/*****************************************************************************/
/******************* Write parameter with code of banner *********************/
/*****************************************************************************/

static void Ban_PutParamBanCod (long BanCod)
  {
   Par_PutHiddenParamLong ("BanCod",BanCod);
  }

/*****************************************************************************/
/******************** Get parameter with code of banner **********************/
/*****************************************************************************/

long Ban_GetParamBanCod (void)
  {
   char LongStr[1+10+1];

   /***** Get parameter with code of banner *****/
   Par_GetParToText ("BanCod",LongStr,1+10);
   return Str_ConvertStrCodToLongCod (LongStr);
  }

/*****************************************************************************/
/******************************* Remove a banner *****************************/
/*****************************************************************************/

void Ban_RemoveBanner (void)
  {
   extern const char *Txt_Banner_X_removed;
   char Query[512];
   struct Banner Ban;

   /***** Get banner code *****/
   if ((Ban.BanCod = Ban_GetParamBanCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of banner is missing.");

   /***** Get data of the banner from database *****/
   Ban_GetDataOfBannerByCod (&Ban);

   /***** Remove banner *****/
   sprintf (Query,"DELETE FROM banners WHERE BanCod='%ld'",Ban.BanCod);
   DB_QueryDELETE (Query,"can not remove a banner");

   /***** Write message to show the change made *****/
   sprintf (Gbl.Message,Txt_Banner_X_removed,
            Ban.ShortName);
   Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);

   /***** Show the form again *****/
   Ban_EditBanners ();
  }

/*****************************************************************************/
/**************************** Show a hidden banner ***************************/
/*****************************************************************************/

void Ban_ShowBanner (void)
  {
   /***** Set banner as visible *****/
   Ban_ShowOrHideBanner (false);
  }

/*****************************************************************************/
/**************************** Hide a visible banner **************************/
/*****************************************************************************/

void Ban_HideBanner (void)
  {
   /***** Set banner as hidden *****/
   Ban_ShowOrHideBanner (true);
  }

/*****************************************************************************/
/*************** Change hiddeness of banner in the database ******************/
/*****************************************************************************/

static void Ban_ShowOrHideBanner (bool Hide)
  {
   extern const char *Txt_The_banner_X_is_now_hidden;
   extern const char *Txt_The_banner_X_is_now_visible;
   char Query[256];
   struct Banner Ban;

   /***** Get banner code *****/
   if ((Ban.BanCod = Ban_GetParamBanCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of banner is missing.");

   /***** Get data of the banner from database *****/
   Ban_GetDataOfBannerByCod (&Ban);

   /***** Mark file as hidden/visible in database *****/
   if (Ban.IsHidden != Hide)
     {
      sprintf (Query,"UPDATE banners SET Hidden='%c'"
		     " WHERE BanCod='%ld'",
	       Hide ? 'Y' :
		      'N',
	       Ban.BanCod);
      DB_QueryUPDATE (Query,"can not change status of a banner in database");
     }

   /***** Write message to show the change made *****/
   sprintf (Gbl.Message,Hide ? Txt_The_banner_X_is_now_hidden :
	                       Txt_The_banner_X_is_now_visible,
	    Ban.ShortName);
   Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);

   /***** Show the form again *****/
   Ban_EditBanners ();
  }

/*****************************************************************************/
/********************* Change the short name of a banner *********************/
/*****************************************************************************/

void Ban_RenameBannerShort (void)
  {
   Ban_RenameBanner (Cns_SHORT_NAME);
  }

/*****************************************************************************/
/********************* Change the full name of a banner **********************/
/*****************************************************************************/

void Ban_RenameBannerFull (void)
  {
   Ban_RenameBanner (Cns_FULL_NAME);
  }

/*****************************************************************************/
/*********************** Change the name of a banner *************************/
/*****************************************************************************/

static void Ban_RenameBanner (Cns_ShortOrFullName_t ShortOrFullName)
  {
   extern const char *Txt_You_can_not_leave_the_name_of_the_banner_X_empty;
   extern const char *Txt_The_banner_X_already_exists;
   extern const char *Txt_The_banner_X_has_been_renamed_as_Y;
   extern const char *Txt_The_name_of_the_banner_X_has_not_changed;
   char Query[512];
   struct Banner *Ban;
   const char *ParamName = NULL;	// Initialized to avoid warning
   const char *FieldName = NULL;	// Initialized to avoid warning
   unsigned MaxLength = 0;		// Initialized to avoid warning
   char *CurrentBanName = NULL;		// Initialized to avoid warning
   char NewBanName[Ban_MAX_LENGTH_FULL_NAME+1];

   Ban = &Gbl.Banners.EditingBan;
   switch (ShortOrFullName)
     {
      case Cns_SHORT_NAME:
         ParamName = "ShortName";
         FieldName = "ShortName";
         MaxLength = Ban_MAX_LENGTH_SHORT_NAME;
         CurrentBanName = Ban->ShortName;
         break;
      case Cns_FULL_NAME:
         ParamName = "FullName";
         FieldName = "FullName";
         MaxLength = Ban_MAX_LENGTH_FULL_NAME;
         CurrentBanName = Ban->FullName;
         break;
     }

   /***** Get parameters from form *****/
   /* Get the code of the banner */
   if ((Ban->BanCod = Ban_GetParamBanCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of banner is missing.");

   /* Get the new name for the banner */
   Par_GetParToText (ParamName,NewBanName,MaxLength);

   /***** Get from the database the old names of the banner *****/
   Ban_GetDataOfBannerByCod (Ban);

   /***** Check if new name is empty *****/
   if (!NewBanName[0])
     {
      sprintf (Gbl.Message,Txt_You_can_not_leave_the_name_of_the_banner_X_empty,
               CurrentBanName);
      Lay_ShowAlert (Lay_WARNING,Gbl.Message);
     }
   else
     {
      /***** Check if old and new names are the same (this happens when user press enter with no changes in the form) *****/
      if (strcmp (CurrentBanName,NewBanName))	// Different names
        {
         /***** If banner was in database... *****/
         if (Ban_CheckIfBannerNameExists (ParamName,NewBanName,Ban->BanCod))
           {
            sprintf (Gbl.Message,Txt_The_banner_X_already_exists,
                     NewBanName);
            Lay_ShowAlert (Lay_WARNING,Gbl.Message);
           }
         else
           {
            /* Update the table changing old name by new name */
            sprintf (Query,"UPDATE banners SET %s='%s' WHERE BanCod='%ld'",
                     FieldName,NewBanName,Ban->BanCod);
            DB_QueryUPDATE (Query,"can not update the name of a banner");

            /***** Write message to show the change made *****/
            sprintf (Gbl.Message,Txt_The_banner_X_has_been_renamed_as_Y,
                     CurrentBanName,NewBanName);
            Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
           }
        }
      else	// The same name
        {
         sprintf (Gbl.Message,Txt_The_name_of_the_banner_X_has_not_changed,
                  CurrentBanName);
         Lay_ShowAlert (Lay_INFO,Gbl.Message);
        }
     }

   /***** Show the form again *****/
   strcpy (CurrentBanName,NewBanName);
   Ban_EditBanners ();
  }

/*****************************************************************************/
/********************* Check if the name of banner exists ********************/
/*****************************************************************************/

static bool Ban_CheckIfBannerNameExists (const char *FieldName,const char *Name,long BanCod)
  {
   char Query[512];

   /***** Get number of banners with a name from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM banners WHERE %s='%s' AND BanCod<>'%ld'",
            FieldName,Name,BanCod);
   return (DB_QueryCOUNT (Query,"can not check if the name of a banner already existed") != 0);
  }

/*****************************************************************************/
/*********************** Change the image of a banner ************************/
/*****************************************************************************/

void Ban_ChangeBannerImg (void)
  {
   extern const char *Txt_The_new_image_is_X;
   extern const char *Txt_You_can_not_leave_the_image_empty;
   struct Banner *Ban;
   char Query[256+Ban_MAX_LENGTH_IMAGE];
   char NewImg[Ban_MAX_LENGTH_IMAGE+1];

   Ban = &Gbl.Banners.EditingBan;

   /***** Get parameters from form *****/
   /* Get the code of the banner */
   if ((Ban->BanCod = Ban_GetParamBanCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of banner is missing.");

   /* Get the new WWW for the banner */
   Par_GetParToText ("Img",NewImg,Ban_MAX_LENGTH_IMAGE);

   /***** Check if new image is empty *****/
   if (NewImg[0])
     {
      /* Update the table changing old image by new image */
      sprintf (Query,"UPDATE banners SET Img='%s' WHERE BanCod='%ld'",
               NewImg,Ban->BanCod);
      DB_QueryUPDATE (Query,"can not update the image of a banner");

      /***** Write message to show the change made *****/
      sprintf (Gbl.Message,Txt_The_new_image_is_X,
               NewImg);
      Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
     }
   else
     Lay_ShowAlert (Lay_WARNING,Txt_You_can_not_leave_the_image_empty);

   /***** Show the form again *****/
   strcpy (Ban->Img,NewImg);
   Ban_EditBanners ();
  }

/*****************************************************************************/
/*********************** Change the WWW of a banner **************************/
/*****************************************************************************/

void Ban_ChangeBannerWWW (void)
  {
   extern const char *Txt_The_new_web_address_is_X;
   extern const char *Txt_You_can_not_leave_the_web_address_empty;
   struct Banner *Ban;
   char Query[256+Cns_MAX_LENGTH_WWW];
   char NewWWW[Cns_MAX_LENGTH_WWW+1];

   Ban = &Gbl.Banners.EditingBan;

   /***** Get parameters from form *****/
   /* Get the code of the banner */
   if ((Ban->BanCod = Ban_GetParamBanCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of banner is missing.");

   /* Get the new WWW for the banner */
   Par_GetParToText ("WWW",NewWWW,Cns_MAX_LENGTH_WWW);

   /***** Check if new WWW is empty *****/
   if (NewWWW[0])
     {
      /* Update the table changing old WWW by new WWW */
      sprintf (Query,"UPDATE banners SET WWW='%s' WHERE BanCod='%ld'",
               NewWWW,Ban->BanCod);
      DB_QueryUPDATE (Query,"can not update the web of a banner");

      /***** Write message to show the change made *****/
      sprintf (Gbl.Message,Txt_The_new_web_address_is_X,
               NewWWW);
      Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
     }
   else
     Lay_ShowAlert (Lay_WARNING,Txt_You_can_not_leave_the_web_address_empty);

   /***** Show the form again *****/
   strcpy (Ban->WWW,NewWWW);
   Ban_EditBanners ();
  }

/*****************************************************************************/
/********************* Put a form to create a new banner *********************/
/*****************************************************************************/

static void Ban_PutFormToCreateBanner (void)
  {
   extern const char *Txt_New_banner;
   extern const char *Txt_Create_banner;
   struct Banner *Ban;

   Ban = &Gbl.Banners.EditingBan;

   /***** Start form *****/
   Act_FormStart (ActNewBan);

   /***** Start of frame *****/
   Lay_StartRoundFrameTable (NULL,2,Txt_New_banner);

   /***** Write heading *****/
   Ban_PutHeadBanners ();

   /***** Banner code *****/
   fprintf (Gbl.F.Out,"<tr>"
                      "<td class=\"BM\"></td>"
                      "<td class=\"BM\"></td>"
                      "<td></td>");

   /***** Banner short name *****/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
                      "<input type=\"text\" name=\"ShortName\""
                      " size=\"12\" maxlength=\"%u\" value=\"%s\" />"
                      "</td>",
            Ban_MAX_LENGTH_SHORT_NAME,Ban->ShortName);

   /***** Banner full name *****/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
                      "<input type=\"text\" name=\"FullName\""
                      " size=\"24\" maxlength=\"%u\" value=\"%s\" />"
                      "</td>",
            Ban_MAX_LENGTH_FULL_NAME,Ban->FullName);

   /***** Banner image *****/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
                      "<input type=\"text\" name=\"Img\""
                      " size=\"12\" maxlength=\"%u\" value=\"%s\" />"
                      "</td>",
            Ban_MAX_LENGTH_IMAGE,Ban->Img);

   /***** Banner WWW *****/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
                      "<input type=\"text\" name=\"WWW\""
                      " size=\"24\" maxlength=\"%u\" value=\"%s\" />"
                      "</td>"
                      "</tr>",
            Cns_MAX_LENGTH_WWW,Ban->WWW);

   /***** Send button and end frame *****/
   Lay_EndRoundFrameTableWithButton (Lay_CREATE_BUTTON,Txt_Create_banner);

   /***** End of form *****/
   Act_FormEnd ();
  }

/*****************************************************************************/
/******************** Write header with fields of a banner *******************/
/*****************************************************************************/

static void Ban_PutHeadBanners (void)
  {
   extern const char *Txt_Code;
   extern const char *Txt_Short_name;
   extern const char *Txt_Full_name;
   extern const char *Txt_Image;
   extern const char *Txt_WWW;

   fprintf (Gbl.F.Out,"<tr>"
                      "<th class=\"BM\"></th>"
                      "<th class=\"BM\"></th>"
                      "<th class=\"RIGHT_MIDDLE\">%s</th>"
                      "<th class=\"LEFT_MIDDLE\">%s</th>"
                      "<th class=\"LEFT_MIDDLE\">%s</th>"
                      "<th class=\"LEFT_MIDDLE\">%s</th>"
                      "<th class=\"LEFT_MIDDLE\">%s</th>"
                      "</tr>",
            Txt_Code,
            Txt_Short_name,
            Txt_Full_name,
            Txt_Image,
            Txt_WWW);
  }

/*****************************************************************************/
/******************* Receive form to create a new banner *********************/
/*****************************************************************************/

void Ban_RecFormNewBanner (void)
  {
   extern const char *Txt_The_banner_X_already_exists;
   extern const char *Txt_You_must_specify_the_image_of_the_new_banner;
   extern const char *Txt_You_must_specify_the_URL_of_the_new_banner;
   extern const char *Txt_You_must_specify_the_short_name_and_the_full_name_of_the_new_banner;
   struct Banner *Ban;

   Ban = &Gbl.Banners.EditingBan;

   /***** Get parameters from form *****/
   /* Get banner short name */
   Par_GetParToText ("ShortName",Ban->ShortName,Ban_MAX_LENGTH_SHORT_NAME);

   /* Get banner full name */
   Par_GetParToText ("FullName",Ban->FullName,Ban_MAX_LENGTH_FULL_NAME);

   /* Get banner image */
   Par_GetParToText ("Img",Ban->Img,Ban_MAX_LENGTH_IMAGE);

   /* Get banner URL */
   Par_GetParToText ("WWW",Ban->WWW,Cns_MAX_LENGTH_WWW);

   if (Ban->ShortName[0] && Ban->FullName[0])	// If there's a banner name
     {
      /***** If name of banner was in database... *****/
      if (Ban_CheckIfBannerNameExists ("ShortName",Ban->ShortName,-1L))
        {
         sprintf (Gbl.Message,Txt_The_banner_X_already_exists,
                  Ban->ShortName);
         Lay_ShowAlert (Lay_WARNING,Gbl.Message);
        }
      else if (Ban_CheckIfBannerNameExists ("FullName",Ban->FullName,-1L))
        {
         sprintf (Gbl.Message,Txt_The_banner_X_already_exists,
                  Ban->FullName);
         Lay_ShowAlert (Lay_WARNING,Gbl.Message);
        }
      else if (!Ban->Img[0])
         Lay_ShowAlert (Lay_WARNING,Txt_You_must_specify_the_image_of_the_new_banner);
      else if (!Ban->WWW[0])
         Lay_ShowAlert (Lay_WARNING,Txt_You_must_specify_the_URL_of_the_new_banner);
      else	// Add new banner to database
         Ban_CreateBanner (Ban);
     }
   else	// If there is not a banner name
      Lay_ShowAlert (Lay_WARNING,Txt_You_must_specify_the_short_name_and_the_full_name_of_the_new_banner);

   /***** Show the form again *****/
   Ban_EditBanners ();
  }

/*****************************************************************************/
/**************************** Create a new banner ****************************/
/*****************************************************************************/

static void Ban_CreateBanner (struct Banner *Ban)
  {
   extern const char *Txt_Created_new_banner_X;
   char Query[256 + Ban_MAX_LENGTH_SHORT_NAME + Ban_MAX_LENGTH_FULL_NAME + Ban_MAX_LENGTH_IMAGE + Cns_MAX_LENGTH_WWW];

   /***** Create a new banner *****/
   sprintf (Query,"INSERT INTO banners (Hidden,ShortName,FullName,Img,WWW)"
	          " VALUES ('N','%s','%s','%s','%s')",
            Ban->ShortName,Ban->FullName,Ban->Img,Ban->WWW);
   DB_QueryINSERT (Query,"can not create banner");

   /***** Write success message *****/
   sprintf (Gbl.Message,Txt_Created_new_banner_X,
            Ban->ShortName);
   Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
  }

/*****************************************************************************/
/************************* Write menu with some banners **********************/
/*****************************************************************************/

void Ban_WriteMenuWithBanners (void)
  {
   char Query[256];
   unsigned NumBan;

   /***** Get random banner *****/
   sprintf (Query,"SELECT BanCod,Hidden,ShortName,FullName,Img,WWW"
	          " FROM banners"
	          " WHERE Hidden='N'"
	          " ORDER BY RAND(%lu) LIMIT %u",
	    (unsigned long) (Gbl.TimeStartExecution / Cfg_TIME_TO_CHANGE_BANNER),
	    Cfg_NUMBER_OF_BANNERS);	// The banner(s) will change once in a while
   Ban_GetListBanners (Query);

   /***** Write all the banners *****/
   for (NumBan = 0;
	NumBan < Gbl.Banners.Num;
	NumBan++)
     {
      /* Write data of this banner */
      fprintf (Gbl.F.Out,"<tr>"
                         "<td class=\"LEFT_MIDDLE\" style=\"width:150px;\">");
      Act_FormStart (ActClkBan);
      Ban_PutParamBanCod (Gbl.Banners.Lst[NumBan].BanCod);
      Par_PutHiddenParamString ("URL",Gbl.Banners.Lst[NumBan].WWW);
      fprintf (Gbl.F.Out,"<a href=\"\" title=\"%s\""
	                 " onclick=\"document.getElementById('%s').submit();\">"
                         "<img src=\"%s/%s/%s\""
                         " alt=\"%s\" title=\"%s\""
                         " class=\"BANNER\" />"
                         "</a>",
	       Gbl.Banners.Lst[NumBan].FullName,
	       Gbl.FormId,
               Cfg_HTTPS_URL_SWAD_PUBLIC,Cfg_FOLDER_BANNER,
               Gbl.Banners.Lst[NumBan].Img,
               Gbl.Banners.Lst[NumBan].ShortName,
               Gbl.Banners.Lst[NumBan].FullName);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>"
                         "</tr>");
     }

   /***** Free list of banners *****/
   Ban_FreeListBanners ();
  }

/*****************************************************************************/
/************************* Go to a banner when clicked ***********************/
/*****************************************************************************/

void Ban_ClickOnBanner (void)
  {
   struct Banner Ban;

   /***** Get banner code *****/
   if ((Ban.BanCod = Ban_GetParamBanCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of banner is missing.");

   /***** Get data of the banner from database *****/
   Ban_GetDataOfBannerByCod (&Ban);

   /***** Set banner clicked in order to log it *****/
   Gbl.Banners.BanCodClicked = Ban.BanCod;

   /***** Download the file *****/
   fprintf (stdout,"Location: %s\n\n",Ban.WWW);
   // TODO: Put headers Content-type and Content-disposition:
   // See: http://stackoverflow.com/questions/381954/how-do-i-fix-firefox-trying-to-save-image-as-htm
   // http://elouai.com/force-download.php
   Gbl.Layout.HTMLStartWritten =
   Gbl.Layout.TablEndWritten   =
   Gbl.Layout.HTMLEndWritten   = true;	// Don't write HTML at all
  }

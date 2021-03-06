// swad_degree_type.c: degree types

/*
    SWAD (Shared Workspace At a Distance),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2017 Antonio Ca�as Vargas

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

#include <ctype.h>		// For isprint, isspace, etc.
#include <linux/stddef.h>	// For NULL
#include <stdbool.h>		// For boolean type
#include <stdio.h>		// For fprintf, etc.
#include <stdlib.h>		// For exit, system, calloc, free, etc.
#include <string.h>		// For string functions
#include <mysql/mysql.h>	// To access MySQL databases

#include "swad_config.h"
#include "swad_database.h"
#include "swad_degree.h"
#include "swad_degree_type.h"
#include "swad_global.h"
#include "swad_parameter.h"

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

extern struct Globals Gbl;

/*****************************************************************************/
/*************************** Public constants ********************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private types *********************************/
/*****************************************************************************/

/*****************************************************************************/
/**************************** Private constants ******************************/
/*****************************************************************************/

/*****************************************************************************/
/**************************** Private prototypes *****************************/
/*****************************************************************************/

static void DT_ListDegreeTypes (void);
static void DT_EditDegreeTypes (void);
static void DT_ListDegreeTypesForSeeing (void);
static void DT_PutIconToEditDegTypes (void);
static void DT_ListDegreeTypesForEdition (void);

static void DT_PutFormToCreateDegreeType (void);
static void DT_PutHeadDegreeTypesForSeeing (void);
static void DT_PutHeadDegreeTypesForEdition (void);
static void DT_CreateDegreeType (struct DegreeType *DegTyp);

static void DT_PutParamOtherDegTypCod (long DegTypCod);

static unsigned DT_CountNumDegsOfType (long DegTypCod);
static void DT_RemoveDegreeTypeCompletely (long DegTypCod);
static bool DT_CheckIfDegreeTypeNameExists (const char *DegTypName,long DegTypCod);

/*****************************************************************************/
/************** Show selector of degree types for statistics *****************/
/*****************************************************************************/

void DT_WriteSelectorDegreeTypes (void)
  {
   extern const char *Txt_Any_type_of_degree;
   unsigned NumDegTyp;

   /***** Form to select degree types *****/
   /* Get list of degree types */
   DT_GetListDegreeTypes ();

   /* List degree types */
   fprintf (Gbl.F.Out,"<select id=\"OthDegTypCod\" name=\"OthDegTypCod\""
	              " onchange=\"document.getElementById('%s').submit();\">",
            Gbl.Form.Id);

   fprintf (Gbl.F.Out,"<option value=\"-1\"");
   if (Gbl.Stat.DegTypCod == -1L)
      fprintf (Gbl.F.Out," selected=\"selected\"");
   fprintf (Gbl.F.Out,">%s</option>",Txt_Any_type_of_degree);

   for (NumDegTyp = 0;
	NumDegTyp < Gbl.Degs.DegTypes.Num;
	NumDegTyp++)
     {
      fprintf (Gbl.F.Out,"<option value=\"%ld\"",Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypCod );
      if (Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypCod  == Gbl.Stat.DegTypCod)
         fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out,">%s</option>",Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypName);
     }

   fprintf (Gbl.F.Out,"</select>");

   /***** Free list of degree types *****/
   DT_FreeListDegreeTypes ();
  }

/*****************************************************************************/
/***************************** Show degree types *****************************/
/*****************************************************************************/

void DT_SeeDegreeTypes (void)
  {
   /***** Get list of degree types *****/
   DT_GetListDegreeTypes ();

   /***** List degree types *****/
   DT_ListDegreeTypes ();

   /***** Free list of degree types *****/
   DT_FreeListDegreeTypes ();
  }

/*****************************************************************************/
/********************** Request edition of degree types **********************/
/*****************************************************************************/

void DT_ReqEditDegreeTypes (void)
  {
   /***** Get list of degree types *****/
   DT_GetListDegreeTypes ();

   /***** Put form to edit degree types *****/
   DT_EditDegreeTypes ();

   /***** Free list of degree types *****/
   DT_FreeListDegreeTypes ();
  }

/*****************************************************************************/
/***************************** List degree types *****************************/
/*****************************************************************************/

static void DT_ListDegreeTypes (void)
  {
   extern const char *Txt_There_are_no_types_of_degree;

   if (Gbl.Degs.DegTypes.Num)
      DT_ListDegreeTypesForSeeing ();
   else
      Lay_ShowAlert (Lay_INFO,Txt_There_are_no_types_of_degree);
  }

/*****************************************************************************/
/************************ Put forms to edit degree types *********************/
/*****************************************************************************/

static void DT_EditDegreeTypes (void)
  {
   extern const char *Txt_There_are_no_types_of_degree;

   if (!Gbl.Degs.DegTypes.Num)
      /***** Help message *****/
      Lay_ShowAlert (Lay_INFO,Txt_There_are_no_types_of_degree);

   /***** Put a form to create a new degree type *****/
   DT_PutFormToCreateDegreeType ();

   /***** Forms to edit current degree types *****/
   if (Gbl.Degs.DegTypes.Num)
      DT_ListDegreeTypesForEdition ();
  }

/*****************************************************************************/
/******************* List current degree types for seeing ********************/
/*****************************************************************************/

static void DT_ListDegreeTypesForSeeing (void)
  {
   extern const char *Hlp_SYSTEM_Studies;
   extern const char *Txt_Types_of_degree;
   extern const char *Txt_TYPES_OF_DEGREE_With_degrees;
   extern const char *Txt_TYPES_OF_DEGREE_Without_degrees;
   unsigned NumDegTyp;
   const char *BgColor;

   /***** Write heading *****/
   Lay_StartRoundFrameTable (NULL,Txt_Types_of_degree,
                             Gbl.Usrs.Me.LoggedRole == Rol_SYS_ADM ? DT_PutIconToEditDegTypes :
                                                                     NULL,
                             Hlp_SYSTEM_Studies,2);
   DT_PutHeadDegreeTypesForSeeing ();

   /***** List degree types with forms for edition *****/
   for (NumDegTyp = 0;
	NumDegTyp < Gbl.Degs.DegTypes.Num;
	NumDegTyp++)
     {
      BgColor = (Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypCod ==
	         Gbl.CurrentDegTyp.DegTyp.DegTypCod) ? "LIGHT_BLUE" :
                                                       Gbl.ColorRows[Gbl.RowEvenOdd];

      /* Put green tip if degree type has degrees */
      fprintf (Gbl.F.Out,"<tr>"
                         "<td class=\"%s\">"
                         "<img src=\"%s/%s16x16.gif\""
                         " alt=\"%s\" title=\"%s\""
                         " class=\"ICO20x20\" />"
                         "</td>",
               BgColor,
               Gbl.Prefs.IconsURL,
               Gbl.Degs.DegTypes.Lst[NumDegTyp].NumDegs ? "ok_green" :
        	                                          "tr",
               Gbl.Degs.DegTypes.Lst[NumDegTyp].NumDegs ? Txt_TYPES_OF_DEGREE_With_degrees :
                                                          Txt_TYPES_OF_DEGREE_Without_degrees,
               Gbl.Degs.DegTypes.Lst[NumDegTyp].NumDegs ? Txt_TYPES_OF_DEGREE_With_degrees :
                                                          Txt_TYPES_OF_DEGREE_Without_degrees);

      /* Name of degree type */
      fprintf (Gbl.F.Out,"<td class=\"DAT LEFT_MIDDLE %s\">"
	                 "%s"
	                 "</td>",
               BgColor,Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypName);

      /* Number of degrees of this type */
      fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_MIDDLE %s\">"
	                 "%u"
	                 "</td>"
                         "</tr>",
               BgColor,Gbl.Degs.DegTypes.Lst[NumDegTyp].NumDegs);

      Gbl.RowEvenOdd = 1 - Gbl.RowEvenOdd;
     }

   /***** End table *****/
   fprintf (Gbl.F.Out,"</table>");
   Lay_EndRoundFrame ();
  }

/*****************************************************************************/
/******************* Put link (form) to edit degree types ********************/
/*****************************************************************************/

static void DT_PutIconToEditDegTypes (void)
  {
   extern const char *Txt_Edit;

   Lay_PutContextualLink (ActEdiDegTyp,NULL,
                          "edit64x64.png",
                          Txt_Edit,NULL,
                          NULL);
  }

/*****************************************************************************/
/******************* List current degree types for edition *******************/
/*****************************************************************************/

static void DT_ListDegreeTypesForEdition (void)
  {
   extern const char *Hlp_SYSTEM_Studies_edit;
   extern const char *Txt_Types_of_degree;
   unsigned NumDegTyp;

   /***** Write heading *****/
   Lay_StartRoundFrameTable (NULL,Txt_Types_of_degree,
                             NULL,Hlp_SYSTEM_Studies_edit,2);
   DT_PutHeadDegreeTypesForEdition ();

   /***** List degree types with forms for edition *****/
   for (NumDegTyp = 0;
	NumDegTyp < Gbl.Degs.DegTypes.Num;
	NumDegTyp++)
     {
      /* Put icon to remove degree type */
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td class=\"BM\">");
      if (Gbl.Degs.DegTypes.Lst[NumDegTyp].NumDegs)	// Degree type has degrees ==> deletion forbidden
         Lay_PutIconRemovalNotAllowed ();
      else
        {
         Act_FormStart (ActRemDegTyp);
         DT_PutParamOtherDegTypCod (Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypCod);
         Lay_PutIconRemove ();
         Act_FormEnd ();
        }

      /* Degree type code */
      fprintf (Gbl.F.Out,"</td>"
	                 "<td class=\"DAT CENTER_MIDDLE\">"
	                 "%ld"
	                 "</td>",
               Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypCod);

      /* Name of degree type */
      fprintf (Gbl.F.Out,"<td class=\"LEFT_MIDDLE\">");
      Act_FormStart (ActRenDegTyp);
      DT_PutParamOtherDegTypCod (Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"DegTypName\""
	                 " size=\"25\" maxlength=\"%u\" value=\"%s\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Deg_MAX_CHARS_DEGREE_TYPE_NAME,
               Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypName,
               Gbl.Form.Id);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Number of degrees of this type */
      fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_MIDDLE\">"
	                 "%u"
	                 "</td>"
                         "</tr>",
               Gbl.Degs.DegTypes.Lst[NumDegTyp].NumDegs);
     }

   Lay_EndRoundFrameTable ();
  }

/*****************************************************************************/
/******************** Put a form to create a new degree type *****************/
/*****************************************************************************/

static void DT_PutFormToCreateDegreeType (void)
  {
   extern const char *Hlp_SYSTEM_Studies_edit;
   extern const char *Txt_New_type_of_degree;
   extern const char *Txt_Type_of_degree;
   extern const char *Txt_Create_type_of_degree;

   /***** Start form *****/
   Act_FormStart (ActNewDegTyp);

   /***** Start of frame *****/
   Lay_StartRoundFrameTable (NULL,Txt_New_type_of_degree,
                             NULL,Hlp_SYSTEM_Studies_edit,2);

   /***** Write heading *****/
   fprintf (Gbl.F.Out,"<tr>"
                      "<th class=\"CENTER_MIDDLE\">"
                      "%s"
                      "</th>"
                      "</tr>",
            Txt_Type_of_degree);

   /***** Degree type name *****/
   fprintf (Gbl.F.Out,"<tr>"
                      "<td class=\"LEFT_MIDDLE\">"
                      "<input type=\"text\" name=\"DegTypName\""
                      " size=\"25\" maxlength=\"%u\" value=\"%s\""
                      " required=\"required\" />"
                     "</td>",
            Deg_MAX_CHARS_DEGREE_TYPE_NAME,Gbl.Degs.EditingDegTyp.DegTypName);

   /***** Send button and end frame *****/
   Lay_EndRoundFrameTableWithButton (Lay_CREATE_BUTTON,Txt_Create_type_of_degree);

   /***** End form *****/
   Act_FormEnd ();
  }

/*****************************************************************************/
/***************** Write header with fields of a degree type *****************/
/*****************************************************************************/

static void DT_PutHeadDegreeTypesForSeeing (void)
  {
   extern const char *Txt_Type_of_degree;
   extern const char *Txt_Degrees;

   fprintf (Gbl.F.Out,"<tr>"
                      "<th class=\"BM\"></th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"RIGHT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "</tr>",
            Txt_Type_of_degree,
            Txt_Degrees);
  }

/*****************************************************************************/
/***************** Write header with fields of a degree type *****************/
/*****************************************************************************/

static void DT_PutHeadDegreeTypesForEdition (void)
  {
   extern const char *Txt_Code;
   extern const char *Txt_Type_of_degree;
   extern const char *Txt_Degrees;

   fprintf (Gbl.F.Out,"<tr>"
                      "<th class=\"BM\"></th>"
                      "<th class=\"CENTER_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"CENTER_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"RIGHT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "</tr>",
            Txt_Code,
            Txt_Type_of_degree,
            Txt_Degrees);
  }

/*****************************************************************************/
/************************** Create a new degree type *************************/
/*****************************************************************************/

static void DT_CreateDegreeType (struct DegreeType *DegTyp)
  {
   extern const char *Txt_Created_new_type_of_degree_X;
   char Query[128 + Deg_MAX_BYTES_DEGREE_TYPE_NAME];

   /***** Create a new degree type *****/
   sprintf (Query,"INSERT INTO deg_types SET DegTypName='%s'",
            DegTyp->DegTypName);
   DB_QueryINSERT (Query,"can not create a new type of degree");

   /***** Write success message *****/
   sprintf (Gbl.Message,Txt_Created_new_type_of_degree_X,
            DegTyp->DegTypName);
   Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
  }

/*****************************************************************************/
/**************** Create a list with all the degree types ********************/
/*****************************************************************************/

void DT_GetListDegreeTypes (void)
  {
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRow;

   /***** Get types of degree from database *****/
   sprintf (Query,"(SELECT deg_types.DegTypCod,deg_types.DegTypName AS DegTypName,"
                  " COUNT(degrees.DegCod)"
                  " FROM deg_types,degrees WHERE deg_types.DegTypCod=degrees.DegTypCod"
                  " GROUP BY degrees.DegTypCod)"
                  " UNION "
                  "(SELECT DegTypCod,DegTypName,'0'"
                  " FROM deg_types WHERE DegTypCod NOT IN (SELECT DegTypCod FROM degrees))"
                  " ORDER BY DegTypName");
   Gbl.Degs.DegTypes.Num = (unsigned) DB_QuerySELECT (Query,&mysql_res,"can not get types of degree");

   /***** Get degree types *****/
   if (Gbl.Degs.DegTypes.Num)
     {
      /***** Create a list of degree types *****/
      if ((Gbl.Degs.DegTypes.Lst = (struct DegreeType *) calloc (Gbl.Degs.DegTypes.Num,sizeof (struct DegreeType))) == NULL)
         Lay_ShowErrorAndExit ("Not enough memory to store types of degree.");

      /***** Get degree types *****/
      for (NumRow = 0;
	   NumRow < Gbl.Degs.DegTypes.Num;
	   NumRow++)
        {
         /* Get next degree type */
         row = mysql_fetch_row (mysql_res);

         /* Get degree type code (row[0]) */
         if ((Gbl.Degs.DegTypes.Lst[NumRow].DegTypCod = Str_ConvertStrCodToLongCod (row[0])) < 0)
            Lay_ShowErrorAndExit ("Wrong code of type of degree.");

         /* Get degree type name (row[1]) */
         Str_Copy (Gbl.Degs.DegTypes.Lst[NumRow].DegTypName,row[1],
                   Deg_MAX_BYTES_DEGREE_TYPE_NAME);

         /* Number of degrees of this type (row[2]) */
         if (sscanf (row[2],"%u",&Gbl.Degs.DegTypes.Lst[NumRow].NumDegs) != 1)
            Lay_ShowErrorAndExit ("Error when getting number of degrees of a type");
        }
     }

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/********* Free list of degree types and list of degrees of each type ********/
/*****************************************************************************/

void DT_FreeListDegreeTypes (void)
  {
   /***** Free memory used by the list of degree types *****/
   if (Gbl.Degs.DegTypes.Lst)
     {
      free ((void *) Gbl.Degs.DegTypes.Lst);
      Gbl.Degs.DegTypes.Lst = NULL;
      Gbl.Degs.DegTypes.Num = 0;
     }
  }

/*****************************************************************************/
/***************** Receive form to create a new degree type ******************/
/*****************************************************************************/

void DT_RecFormNewDegreeType (void)
  {
   extern const char *Txt_The_type_of_degree_X_already_exists;
   extern const char *Txt_You_must_specify_the_name_of_the_new_type_of_degree;
   struct DegreeType *DegTyp;

   DegTyp = &Gbl.Degs.EditingDegTyp;

   /***** Get parameters from form *****/
   /* Get the name of degree type */
   Par_GetParToText ("DegTypName",DegTyp->DegTypName,Deg_MAX_BYTES_DEGREE_TYPE_NAME);

   if (DegTyp->DegTypName[0])	// If there's a degree type name
     {
      /***** If name of degree type was in database... *****/
      if (DT_CheckIfDegreeTypeNameExists (DegTyp->DegTypName,-1L))
        {
         sprintf (Gbl.Message,Txt_The_type_of_degree_X_already_exists,
                  DegTyp->DegTypName);
         Lay_ShowAlert (Lay_WARNING,Gbl.Message);
        }
      else	// Add new degree type to database
         DT_CreateDegreeType (DegTyp);
     }
   else	// If there is not a degree type name
     {
      sprintf (Gbl.Message,"%s",Txt_You_must_specify_the_name_of_the_new_type_of_degree);
      Lay_ShowAlert (Lay_WARNING,Gbl.Message);
     }

   /***** Show the form again *****/
   DT_ReqEditDegreeTypes ();
  }

/*****************************************************************************/
/**************************** Remove a degree type ***************************/
/*****************************************************************************/

void DT_RemoveDegreeType (void)
  {
   extern const char *Txt_To_remove_a_type_of_degree_you_must_first_remove_all_degrees_of_that_type;
   extern const char *Txt_Type_of_degree_X_removed;
   struct DegreeType DegTyp;

   /***** Get the code of the degree type *****/
   if ((DegTyp.DegTypCod = DT_GetParamOtherDegTypCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of type of degree is missing.");

   /***** Get data of the degree type from database *****/
   if (!DT_GetDataOfDegreeTypeByCod (&DegTyp))
      Lay_ShowErrorAndExit ("Code of type of degree not found.");

   /***** Check if this degree type has degrees *****/
   if (DegTyp.NumDegs)	// Degree type has degrees ==> don't remove
      Lay_ShowAlert (Lay_WARNING,Txt_To_remove_a_type_of_degree_you_must_first_remove_all_degrees_of_that_type);
   else	// Degree type has no degrees ==> remove it
     {
      /***** Remove degree type *****/
      DT_RemoveDegreeTypeCompletely (DegTyp.DegTypCod);

      /***** Write message to show the change made *****/
      sprintf (Gbl.Message,Txt_Type_of_degree_X_removed,
               DegTyp.DegTypName);
      Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
     }

   /***** Show the form again *****/
   DT_ReqEditDegreeTypes ();
  }

/*****************************************************************************/
/***************** Write parameter with code of degree type ******************/
/*****************************************************************************/

static void DT_PutParamOtherDegTypCod (long DegTypCod)
  {
   Par_PutHiddenParamLong ("OthDegTypCod",DegTypCod);
  }

/*****************************************************************************/
/******************* Get parameter with code of degree type ******************/
/*****************************************************************************/

long DT_GetParamOtherDegTypCod (void)
  {
   /***** Get code of degree type *****/
   return Par_GetParToLong ("OthDegTypCod");
  }

/*****************************************************************************/
/**************** Count number of degrees in a degree type ******************/
/*****************************************************************************/

static unsigned DT_CountNumDegsOfType (long DegTypCod)
  {
   char Query[512];

   /***** Get number of degrees of a type from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM degrees WHERE DegTypCod='%ld'",
            DegTypCod);
   return (unsigned) DB_QueryCOUNT (Query,"can not get number of degrees of a type");
  }

/*****************************************************************************/
/****************** Get data of a degree type from its code ******************/
/*****************************************************************************/

bool DT_GetDataOfDegreeTypeByCod (struct DegreeType *DegTyp)
  {
   char Query[512];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRows;
   bool DegTypFound = false;

   if (DegTyp->DegTypCod <= 0)
     {
      DegTyp->DegTypCod = -1L;
      DegTyp->DegTypName[0] = '\0';
      DegTyp->NumDegs = 0;
      return false;
     }

   /***** Get the name of a type of degree from database *****/
   sprintf (Query,"SELECT DegTypName FROM deg_types WHERE DegTypCod='%ld'",
            DegTyp->DegTypCod);
   NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get the name of a type of degree");

   if (NumRows == 1)
     {
      /***** Get data of degree type *****/
      row = mysql_fetch_row (mysql_res);

      /* Get the name of the degree type (row[0]) */
      Str_Copy (DegTyp->DegTypName,row[0],
                Deg_MAX_BYTES_DEGREE_TYPE_NAME);

      /* Count number of degrees of this type */
      DegTyp->NumDegs = DT_CountNumDegsOfType (DegTyp->DegTypCod);

      /* Set return value */
      DegTypFound = true;
     }
   else if (NumRows == 0)
     {
      DegTyp->DegTypCod = -1L;
      DegTyp->DegTypName[0] = '\0';
      DegTyp->NumDegs = 0;
      return false;
     }
   else // NumRows > 1
      Lay_ShowErrorAndExit ("Type of degree repeated in database.");

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);

   return DegTypFound;
  }

/*****************************************************************************/
/******************** Remove a degree type and its degrees *******************/
/*****************************************************************************/

static void DT_RemoveDegreeTypeCompletely (long DegTypCod)
  {
   char Query[512];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRow,NumRows;
   long DegCod;

   /***** Get degrees of a type from database *****/
   sprintf (Query,"SELECT DegCod FROM degrees WHERE DegTypCod='%ld'",
            DegTypCod);
   NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get degrees of a type");

   /* Get degrees of this type */
   for (NumRow = 0;
	NumRow < NumRows;
	NumRow++)
     {
      /* Get next degree */
      row = mysql_fetch_row (mysql_res);

      /* Get degree code (row[0]) */
      if ((DegCod = Str_ConvertStrCodToLongCod (row[0])) < 0)
         Lay_ShowErrorAndExit ("Wrong code of degree.");

      /* Remove degree */
      Deg_RemoveDegreeCompletely (DegCod);
     }

   /* Free structure that stores the query result */
   DB_FreeMySQLResult (&mysql_res);

   /***** Remove the degree type *****/
   sprintf (Query,"DELETE FROM deg_types WHERE DegTypCod='%ld'",DegTypCod);
   DB_QueryDELETE (Query,"can not remove a type of degree");
  }

/*****************************************************************************/
/**************************** Rename a degree type ***************************/
/*****************************************************************************/

void DT_RenameDegreeType (void)
  {
   extern const char *Txt_You_can_not_leave_the_name_of_the_type_of_degree_X_empty;
   extern const char *Txt_The_type_of_degree_X_already_exists;
   extern const char *Txt_The_type_of_degree_X_has_been_renamed_as_Y;
   extern const char *Txt_The_name_of_the_type_of_degree_X_has_not_changed;
   struct DegreeType *DegTyp;
   char Query[1024];
   char NewNameDegTyp[Deg_MAX_BYTES_DEGREE_TYPE_NAME + 1];

   DegTyp = &Gbl.Degs.EditingDegTyp;

   /***** Get parameters from form *****/
   /* Get the code of the degree type */
   if ((DegTyp->DegTypCod = DT_GetParamOtherDegTypCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of type of degree is missing.");

   /* Get the new name for the degree type */
   Par_GetParToText ("DegTypName",NewNameDegTyp,Deg_MAX_BYTES_DEGREE_TYPE_NAME);

   /***** Get from the database the old name of the degree type *****/
   if (!DT_GetDataOfDegreeTypeByCod (DegTyp))
      Lay_ShowErrorAndExit ("Code of type of degree not found.");

   /***** Check if new name is empty *****/
   if (!NewNameDegTyp[0])
     {
      sprintf (Gbl.Message,Txt_You_can_not_leave_the_name_of_the_type_of_degree_X_empty,
               DegTyp->DegTypName);
      Lay_ShowAlert (Lay_WARNING,Gbl.Message);
     }
   else
     {
      /***** Check if old and new names are the same (this happens when user press enter with no changes in the form) *****/
      if (strcmp (DegTyp->DegTypName,NewNameDegTyp))	// Different names
        {
         /***** If degree type was in database... *****/
         if (DT_CheckIfDegreeTypeNameExists (NewNameDegTyp,DegTyp->DegTypCod))
           {
            sprintf (Gbl.Message,Txt_The_type_of_degree_X_already_exists,
                     NewNameDegTyp);
            Lay_ShowAlert (Lay_WARNING,Gbl.Message);
           }
         else
           {
            /* Update the table changing old name by new name */
            sprintf (Query,"UPDATE deg_types SET DegTypName='%s'"
                           " WHERE DegTypCod='%ld'",
                     NewNameDegTyp,DegTyp->DegTypCod);
            DB_QueryUPDATE (Query,"can not update the type of a degree");

            /* Write message to show the change made */
            sprintf (Gbl.Message,Txt_The_type_of_degree_X_has_been_renamed_as_Y,
                     DegTyp->DegTypName,NewNameDegTyp);
            Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
           }
        }
      else	// The same name
        {
         sprintf (Gbl.Message,Txt_The_name_of_the_type_of_degree_X_has_not_changed,
                  NewNameDegTyp);
         Lay_ShowAlert (Lay_INFO,Gbl.Message);
        }
     }

   /***** Show the form again *****/
   Str_Copy (DegTyp->DegTypName,NewNameDegTyp,
             Deg_MAX_BYTES_DEGREE_TYPE_NAME);
   DT_ReqEditDegreeTypes ();
  }

/*****************************************************************************/
/****************** Check if name of degree type exists **********************/
/*****************************************************************************/

static bool DT_CheckIfDegreeTypeNameExists (const char *DegTypName,long DegTypCod)
  {
   char Query[512];

   /***** Get number of degree types with a name from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM deg_types"
                  " WHERE DegTypName='%s' AND DegTypCod<>'%ld'",
            DegTypName,DegTypCod);
   return (DB_QueryCOUNT (Query,"can not check if the name of a type of degree already existed") != 0);
  }

/*****************************************************************************/
/************************ Change the type of a degree ************************/
/*****************************************************************************/

void DT_ChangeDegreeType (void)
  {
   extern const char *Txt_The_type_of_degree_of_the_degree_X_has_changed;
   struct Degree *Deg;
   long NewDegTypCod;
   char Query[512];

   Deg = &Gbl.Degs.EditingDeg;

   /***** Get parameters from form *****/
   /* Get degree code */
   Deg->DegCod = Deg_GetAndCheckParamOtherDegCod ();

   /* Get the new degree type */
   NewDegTypCod = DT_GetParamOtherDegTypCod ();

   /***** Get data of degree *****/
   Deg_GetDataOfDegreeByCod (Deg);

   /***** Update the table of degrees changing old type by new type *****/
   sprintf (Query,"UPDATE degrees SET DegTypCod='%ld' WHERE DegCod='%ld'",
	    NewDegTypCod,Deg->DegCod);
   DB_QueryUPDATE (Query,"can not update the type of a degree");

   /***** Write message to show the change made *****/
   sprintf (Gbl.Message,Txt_The_type_of_degree_of_the_degree_X_has_changed,
	    Deg->FullName);
   Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);

   /***** Put button to go to degree changed *****/
   Deg_PutButtonToGoToDeg (Deg);

   /***** Show the form again *****/
   Gbl.Degs.EditingDegTyp.DegTypCod = NewDegTypCod;
   Deg_EditDegrees ();
  }

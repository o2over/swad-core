// swad_department.c: departments

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

#include <linux/stddef.h>	// For NULL
#include <stdbool.h>		// For boolean type
#include <stdlib.h>		// For calloc
#include <string.h>		// For string functions

#include "swad_constant.h"
#include "swad_database.h"
#include "swad_department.h"
#include "swad_global.h"
#include "swad_institution.h"
#include "swad_parameter.h"
#include "swad_string.h"
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

static void Dpt_GetParamDptOrder (void);
static void Dpt_PutIconToEditDpts (void);
static void Dpt_ListDepartmentsForEdition (void);
static void Dpt_PutParamDptCod (long DptCod);

static void Dpt_RenameDepartment (Cns_ShrtOrFullName_t ShrtOrFullName);
static bool Dpt_CheckIfDepartmentNameExists (const char *FieldName,const char *Name,long DptCod);
static void Dpt_UpdateDegNameDB (long DptCod,const char *FieldName,const char *NewDptName);

static void Dpt_PutFormToCreateDepartment (void);
static void Dpt_PutHeadDepartments (void);
static void Dpt_CreateDepartment (struct Department *Dpt);

/*****************************************************************************/
/************************* List all the departments **************************/
/*****************************************************************************/

void Dpt_SeeDepts (void)
  {
   extern const char *Hlp_INSTITUTION_Departments;
   extern const char *Txt_Departments;
   extern const char *Txt_DEPARTMENTS_HELP_ORDER[2];
   extern const char *Txt_DEPARTMENTS_ORDER[2];
   extern const char *Txt_Other_departments;
   extern const char *Txt_Department_unspecified;
   Dpt_Order_t Order;
   unsigned NumDpt;
   unsigned NumTchsInsWithDpt = 0;	// Number of teachers from the current institution with department
   unsigned NumTchsInOtherDpts;

   if (Gbl.CurrentIns.Ins.InsCod > 0)	// Institution selected
     {
      /***** Get parameter with the type of order in the list of departments *****/
      Dpt_GetParamDptOrder ();

      /***** Get list of departments *****/
      Dpt_GetListDepartments (Gbl.CurrentIns.Ins.InsCod);

      /***** Table head *****/
      Lay_StartRoundFrameTable (NULL,Txt_Departments,
                                Gbl.Usrs.Me.LoggedRole == Rol_SYS_ADM ? Dpt_PutIconToEditDpts :
                        	                                        NULL,
                                Hlp_INSTITUTION_Departments,2);
      fprintf (Gbl.F.Out,"<tr>");
      for (Order = Dpt_ORDER_BY_DEPARTMENT;
	   Order <= Dpt_ORDER_BY_NUM_TCHS;
	   Order++)
	{
	 fprintf (Gbl.F.Out,"<th class=\"LEFT_MIDDLE\">");
	 Act_FormStart (ActSeeDpt);
	 Par_PutHiddenParamUnsigned ("Order",(unsigned) Order);
	 Act_LinkFormSubmit (Txt_DEPARTMENTS_HELP_ORDER[Order],"TIT_TBL",NULL);
	 if (Order == Gbl.Dpts.SelectedOrder)
	    fprintf (Gbl.F.Out,"<u>");
	 fprintf (Gbl.F.Out,"%s",Txt_DEPARTMENTS_ORDER[Order]);
	 if (Order == Gbl.Dpts.SelectedOrder)
	    fprintf (Gbl.F.Out,"</u>");
	 fprintf (Gbl.F.Out,"</a>");
	 Act_FormEnd ();
	 fprintf (Gbl.F.Out,"</th>");
	}
      fprintf (Gbl.F.Out,"</tr>");

      /***** Write all the departments and their nuber of teachers *****/
      for (NumDpt = 0;
	   NumDpt < Gbl.Dpts.Num;
	   NumDpt++)
	{
	 /* Write data of this department */
	 fprintf (Gbl.F.Out,"<tr>"
	                    "<td class=\"LEFT_MIDDLE\">"
			    "<a href=\"%s\" target=\"_blank\" class=\"DAT\">"
			    "%s"
			    "</a>"
			    "</td>"
	                    "<td class=\"DAT RIGHT_MIDDLE\">"
	                    "%u"
	                    "</td>"
	                    "</tr>",
	          Gbl.Dpts.Lst[NumDpt].WWW,
		  Gbl.Dpts.Lst[NumDpt].FullName,
		  Gbl.Dpts.Lst[NumDpt].NumTchs);

	 /* Update number of teachers from the current institution
	    with department */
	 NumTchsInsWithDpt += Gbl.Dpts.Lst[NumDpt].NumTchs;
	}

      /***** Separation row *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td colspan=\"3\" class=\"DAT\">"
			 "&nbsp;"
			 "</td>"
			 "</tr>");

      /***** Write teachers with other department *****/
      NumTchsInOtherDpts = Usr_GetNumTchsCurrentInsInDepartment (0);
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"DAT LEFT_MIDDLE\">"
			 "%s"
			 "</td>"
			 "<td class=\"DAT RIGHT_MIDDLE\">"
			 "%u"
			 "</td>"
			 "</tr>",
	       Txt_Other_departments,NumTchsInOtherDpts);
      NumTchsInsWithDpt += NumTchsInOtherDpts;

      /***** Write teachers with no department *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"DAT LEFT_MIDDLE\">"
			 "%s"
			 "</td>"
			 "<td class=\"DAT RIGHT_MIDDLE\">"
			 "%u"
			 "</td>"
			 "</tr>",
	       Txt_Department_unspecified,
	       Sta_GetTotalNumberOfUsersInCourses (Sco_SCOPE_INS,
					  Rol_TEACHER) - NumTchsInsWithDpt);

      /***** End table *****/
      Lay_EndRoundFrameTable ();

      /***** Free list of departments *****/
      Dpt_FreeListDepartments ();
     }
  }

/*****************************************************************************/
/******** Get parameter with the type or order in list of departments ********/
/*****************************************************************************/

static void Dpt_GetParamDptOrder (void)
  {
   Gbl.Dpts.SelectedOrder = (Dpt_Order_t)
	                    Par_GetParToUnsignedLong ("Order",
                                                      0,
                                                      Dpt_NUM_ORDERS - 1,
                                                      (unsigned long) Dpt_ORDER_DEFAULT);
  }

/*****************************************************************************/
/************************ Put icon to edit departments ***********************/
/*****************************************************************************/

static void Dpt_PutIconToEditDpts (void)
  {
   extern const char *Txt_Edit;

   Lay_PutContextualLink (ActEdiDpt,NULL,
                          "edit64x64.png",
                          Txt_Edit,NULL,
                          NULL);
  }

/*****************************************************************************/
/******* Put forms to edit the departments of the current institution ********/
/*****************************************************************************/
// An institution must be selected

void Dpt_EditDepartments (void)
  {
   extern const char *Txt_There_are_no_departments;

   /***** Check if institution is selected *****/
   if (Gbl.CurrentIns.Ins.InsCod <= 0)
      Lay_ShowErrorAndExit ("No institution selected.");		// This should not happen

   /***** Get list of institutions *****/
   Ins_GetListInstitutions (Gbl.CurrentCty.Cty.CtyCod,Ins_GET_BASIC_DATA);
   if (!Gbl.Inss.Num)
      Lay_ShowErrorAndExit ("There is no list of institutions.");	// This should not happen

   /***** Get list of departments *****/
   Dpt_GetListDepartments (Gbl.CurrentIns.Ins.InsCod);

   if (!Gbl.Dpts.Num)
      /***** Help message *****/
      Lay_ShowAlert (Lay_INFO,Txt_There_are_no_departments);

   /***** Put a form to create a new department *****/
   Dpt_PutFormToCreateDepartment ();

   /***** Forms to edit current departments *****/
   if (Gbl.Dpts.Num)
      Dpt_ListDepartmentsForEdition ();

   /***** Free list of departments *****/
   Dpt_FreeListDepartments ();

   /***** Free list of institutions *****/
   Ins_FreeListInstitutions ();
  }

/*****************************************************************************/
/************************** Get list of departments **************************/
/*****************************************************************************/
// If InsCod <= 0 ==> get all the departments
// If InsCod  > 0 ==> get departments of an institution

void Dpt_GetListDepartments (long InsCod)
  {
   char OrderBySubQuery[256];
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRows;
   unsigned NumDpt;
   struct Department *Dpt;

   /***** Get departments from database *****/
   switch (Gbl.Dpts.SelectedOrder)
     {
      case Dpt_ORDER_BY_DEPARTMENT:
         sprintf (OrderBySubQuery,"FullName");
         break;
      case Dpt_ORDER_BY_NUM_TCHS:
         sprintf (OrderBySubQuery,"NumTchs DESC,FullName");
         break;
     }
   if (InsCod > 0)	// Only the departments of an institution
      sprintf (Query,"(SELECT departments.DptCod,departments.InsCod,"
		     "departments.ShortName,departments.FullName,departments.WWW,"
		     "COUNT(DISTINCT usr_data.UsrCod) AS NumTchs"
		     " FROM departments,usr_data,crs_usr"
		     " WHERE departments.InsCod='%ld'"
		     " AND departments.DptCod=usr_data.DptCod"
		     " AND usr_data.UsrCod=crs_usr.UsrCod"
		     " AND crs_usr.Role='%u'"
		     " GROUP BY departments.DptCod)"
		     " UNION "
		     "(SELECT DptCod,InsCod,ShortName,FullName,WWW,0 AS NumTchs"
		     " FROM departments"
		     " WHERE InsCod='%ld' AND DptCod NOT IN"
		     " (SELECT DISTINCT usr_data.DptCod FROM usr_data,crs_usr"
		     " WHERE crs_usr.Role='%u' AND crs_usr.UsrCod=usr_data.UsrCod))"
		     " ORDER BY %s",
	       InsCod,(unsigned) Rol_TEACHER,
	       InsCod,(unsigned) Rol_TEACHER,
	       OrderBySubQuery);
   else			// All the departments
      sprintf (Query,"(SELECT departments.DptCod,departments.InsCod,"
		     "departments.ShortName,departments.FullName,departments.WWW,"
		     "COUNT(DISTINCT usr_data.UsrCod) AS NumTchs"
		     " FROM departments,usr_data,crs_usr"
		     " WHERE departments.DptCod=usr_data.DptCod"
		     " AND usr_data.UsrCod=crs_usr.UsrCod"
		     " AND crs_usr.Role='%u'"
		     " GROUP BY departments.DptCod)"
		     " UNION "
		     "(SELECT DptCod,InsCod,ShortName,FullName,WWW,0 AS NumTchs"
		     " FROM departments"
		     " WHERE DptCod NOT IN"
		     " (SELECT DISTINCT usr_data.DptCod FROM usr_data,crs_usr"
		     " WHERE crs_usr.Role='%u' AND crs_usr.UsrCod=usr_data.UsrCod))"
		     " ORDER BY %s",
	       (unsigned) Rol_TEACHER,
	       (unsigned) Rol_TEACHER,
	       OrderBySubQuery);
   NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get departments");

   if (NumRows) // Departments found...
     {
      // NumRows should be equal to Deg->NumCourses
      Gbl.Dpts.Num = (unsigned) NumRows;

      /***** Create list with courses in degree *****/
      if ((Gbl.Dpts.Lst = (struct Department *) calloc (NumRows,sizeof (struct Department))) == NULL)
          Lay_ShowErrorAndExit ("Not enough memory to store departments.");

      /***** Get the departments *****/
      for (NumDpt = 0;
	   NumDpt < Gbl.Dpts.Num;
	   NumDpt++)
        {
         Dpt = &(Gbl.Dpts.Lst[NumDpt]);

         /* Get next department */
         row = mysql_fetch_row (mysql_res);

         /* Get department code (row[0]) */
         if ((Dpt->DptCod = Str_ConvertStrCodToLongCod (row[0])) < 0)
            Lay_ShowErrorAndExit ("Wrong code of department.");

         /* Get institution code (row[1]) */
         if ((Dpt->InsCod = Str_ConvertStrCodToLongCod (row[1])) < 0)
            Lay_ShowErrorAndExit ("Wrong code of institution.");

         /* Get the short name of the department (row[2]) */
         Str_Copy (Dpt->ShrtName,row[2],
                   Hie_MAX_BYTES_SHRT_NAME);

         /* Get the full name of the department (row[3]) */
         Str_Copy (Dpt->FullName,row[3],
                   Hie_MAX_BYTES_FULL_NAME);

         /* Get the URL of the department (row[4]) */
         Str_Copy (Dpt->WWW,row[4],
                   Cns_MAX_BYTES_WWW);

         /* Get number of teachers in this department (row[5]) */
         if (sscanf (row[5],"%u",&Dpt->NumTchs) != 1)
            Dpt->NumTchs = 0;
        }
     }
   else
      Gbl.Dpts.Num = 0;

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/************************** Get department full name *************************/
/*****************************************************************************/

void Dpt_GetDataOfDepartmentByCod (struct Department *Dpt)
  {
   extern const char *Txt_Another_department;
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRows;

   /***** Clear data *****/
   Dpt->InsCod = -1L;
   Dpt->ShrtName[0] = Dpt->FullName[0] = Dpt->WWW[0] = '\0';
   Dpt->NumTchs = 0;

   /***** Check if department code is correct *****/
   if (Dpt->DptCod == 0)
     {
      Str_Copy (Dpt->ShrtName,Txt_Another_department,
                Hie_MAX_BYTES_SHRT_NAME);
      Str_Copy (Dpt->FullName,Txt_Another_department,
                Hie_MAX_BYTES_FULL_NAME);
     }
   else if (Dpt->DptCod > 0)
     {
      /***** Get data of a department from database *****/
      sprintf (Query,"(SELECT departments.InsCod,departments.ShortName,departments.FullName,departments.WWW,"
                     "COUNT(DISTINCT usr_data.UsrCod) AS NumTchs"
                     " FROM departments,usr_data,crs_usr"
                     " WHERE departments.DptCod='%ld'"
                     " AND departments.DptCod=usr_data.DptCod"
                     " AND usr_data.UsrCod=crs_usr.UsrCod"
                     " AND crs_usr.Role='%u'"
                     " GROUP BY departments.DptCod)"
                     " UNION "
                     "(SELECT InsCod,ShortName,FullName,WWW,0"
                     " FROM departments"
                     " WHERE DptCod='%ld' AND DptCod NOT IN"
                     " (SELECT DISTINCT usr_data.DptCod FROM usr_data,crs_usr"
                     " WHERE crs_usr.Role='%u' AND crs_usr.UsrCod=usr_data.UsrCod))",
               Dpt->DptCod,(unsigned) Rol_TEACHER,
               Dpt->DptCod,(unsigned) Rol_TEACHER);
      NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get data of a department");

      if (NumRows) // Department found...
        {
         /* Get row */
         row = mysql_fetch_row (mysql_res);

         /* Get the code of the institution (row[0]) */
         Dpt->InsCod = Str_ConvertStrCodToLongCod (row[0]);

         /* Get the short name of the department (row[1]) */
         Str_Copy (Dpt->ShrtName,row[1],
                   Hie_MAX_BYTES_SHRT_NAME);

         /* Get the full name of the department (row[2]) */
         Str_Copy (Dpt->FullName,row[2],
                   Hie_MAX_BYTES_FULL_NAME);

         /* Get the URL of the department (row[3]) */
         Str_Copy (Dpt->WWW,row[3],
                   Cns_MAX_BYTES_WWW);

         /* Get number of teachers in this department (row[4]) */
         if (sscanf (row[4],"%u",&Dpt->NumTchs) != 1)
            Dpt->NumTchs = 0;
        }

      /***** Free structure that stores the query result *****/
      DB_FreeMySQLResult (&mysql_res);
     }
  }

/*****************************************************************************/
/************************** Free list of departments *************************/
/*****************************************************************************/

void Dpt_FreeListDepartments (void)
  {
   if (Gbl.Dpts.Lst)
     {
      /***** Free memory used by the list of courses in degree *****/
      free ((void *) Gbl.Dpts.Lst);
      Gbl.Dpts.Lst = NULL;
      Gbl.Dpts.Num = 0;
     }
  }

/*****************************************************************************/
/************** Get number of departments in an institution ******************/
/*****************************************************************************/

unsigned Dpt_GetNumDepartmentsInInstitution (long InsCod)
  {
   char Query[128];

   /***** Get number of departments in an institution from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM departments"
	          " WHERE InsCod='%ld'",
	    InsCod);
   return (unsigned) DB_QueryCOUNT (Query,"can not get number of departments in an institution");
  }

/*****************************************************************************/
/************************** List all the departments *************************/
/*****************************************************************************/

static void Dpt_ListDepartmentsForEdition (void)
  {
   extern const char *Hlp_INSTITUTION_Departments_edit;
   extern const char *Txt_Departments;
   extern const char *Txt_Another_institution;
   unsigned NumDpt;
   struct Department *Dpt;
   struct Instit Ins;
   unsigned NumIns;

   Lay_StartRoundFrameTable (NULL,Txt_Departments,
                             NULL,Hlp_INSTITUTION_Departments_edit,2);

   /***** Table head *****/
   Dpt_PutHeadDepartments ();

   /***** Write all the departments *****/
   for (NumDpt = 0;
	NumDpt < Gbl.Dpts.Num;
	NumDpt++)
     {
      Dpt = &Gbl.Dpts.Lst[NumDpt];

      /* Get data of institution of this department */
      Ins.InsCod = Dpt->InsCod;
      Ins_GetDataOfInstitutionByCod (&Ins,Ins_GET_BASIC_DATA);

      /* Put icon to remove department */
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td class=\"BM\">");
      if (Dpt->NumTchs)	// Department has teachers ==> deletion forbidden
         Lay_PutIconRemovalNotAllowed ();
      else
        {
         Act_FormStart (ActRemDpt);
         Dpt_PutParamDptCod (Dpt->DptCod);
         Lay_PutIconRemove ();
         Act_FormEnd ();
        }
      fprintf (Gbl.F.Out,"</td>");

      /* Department code */
      fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_MIDDLE\">"
	                 "%ld&nbsp;"
	                 "</td>",
               Dpt->DptCod);

      /* Institution */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActChgDptIns);
      Dpt_PutParamDptCod (Dpt->DptCod);
      fprintf (Gbl.F.Out,"<select name=\"OthInsCod\" style=\"width:62px;\""
	                 "onchange=\"document.getElementById('%s').submit();\">"
                         "<option value=\"0\"",
	       Gbl.Form.Id);
      if (Dpt->InsCod == 0)
         fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out,">%s</option>",Txt_Another_institution);
      for (NumIns = 0;
	   NumIns < Gbl.Inss.Num;
	   NumIns++)
         fprintf (Gbl.F.Out,"<option value=\"%ld\"%s>%s</option>",
                  Gbl.Inss.Lst[NumIns].InsCod,
                  Gbl.Inss.Lst[NumIns].InsCod == Dpt->InsCod ? " selected=\"selected\"" :
                	                                       "",
                  Gbl.Inss.Lst[NumIns].ShrtName);
      fprintf (Gbl.F.Out,"</select>");
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Department short name */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActRenDptSho);
      Dpt_PutParamDptCod (Dpt->DptCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"ShortName\""
	                 " maxlength=\"%u\" value=\"%s\""
                         " class=\"INPUT_SHORT_NAME\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Hie_MAX_CHARS_SHRT_NAME,Dpt->ShrtName,Gbl.Form.Id);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Department full name */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActRenDptFul);
      Dpt_PutParamDptCod (Dpt->DptCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"FullName\""
	                 " maxlength=\"%u\" value=\"%s\""
                         " class=\"INPUT_FULL_NAME\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Hie_MAX_CHARS_FULL_NAME,Dpt->FullName,Gbl.Form.Id);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Department WWW */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActChgDptWWW);
      Dpt_PutParamDptCod (Dpt->DptCod);
      fprintf (Gbl.F.Out,"<input type=\"url\" name=\"WWW\""
	                 " maxlength=\"%u\" value=\"%s\""
                         " class=\"INPUT_WWW\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Cns_MAX_CHARS_WWW,Dpt->WWW,Gbl.Form.Id);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Number of teachers */
      fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_MIDDLE\">"
	                 "%u"
	                 "</td>"
	                 "</tr>",
               Dpt->NumTchs);
     }

   Lay_EndRoundFrameTable ();
  }

/*****************************************************************************/
/****************** Write parameter with code of department ******************/
/*****************************************************************************/

static void Dpt_PutParamDptCod (long DptCod)
  {
   Par_PutHiddenParamLong ("DptCod",DptCod);
  }

/*****************************************************************************/
/******************* Get parameter with code of degree type ******************/
/*****************************************************************************/

long Dpt_GetParamDptCod (void)
  {
   /***** Get code of department *****/
   return Par_GetParToLong ("DptCod");
  }

/*****************************************************************************/
/***************************** Remove a department ***************************/
/*****************************************************************************/

void Dpt_RemoveDepartment (void)
  {
   extern const char *Txt_To_remove_a_department_you_must_first_remove_all_teachers_in_the_department;
   extern const char *Txt_Department_X_removed;
   char Query[512];
   struct Department Dpt;

   /***** Get department code *****/
   if ((Dpt.DptCod = Dpt_GetParamDptCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of department is missing.");

   /***** Get data of the department from database *****/
   Dpt_GetDataOfDepartmentByCod (&Dpt);

   /***** Check if this department has teachers *****/
   if (Dpt.NumTchs)	// Department has teachers ==> don't remove
      Lay_ShowAlert (Lay_WARNING,Txt_To_remove_a_department_you_must_first_remove_all_teachers_in_the_department);
   else	// Department has no teachers ==> remove it
     {
      /***** Remove department *****/
      sprintf (Query,"DELETE FROM departments WHERE DptCod='%ld'",Dpt.DptCod);
      DB_QueryDELETE (Query,"can not remove a department");

      /***** Write message to show the change made *****/
      sprintf (Gbl.Message,Txt_Department_X_removed,
               Dpt.FullName);
      Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
     }

   /***** Show the form again *****/
   Dpt_EditDepartments ();
  }

/*****************************************************************************/
/****************** Change the institution of a department *******************/
/*****************************************************************************/

void Dpt_ChangeDepartIns (void)
  {
   extern const char *Txt_The_institution_of_the_department_has_changed;
   struct Department *Dpt;
   char Query[512];

   Dpt = &Gbl.Dpts.EditingDpt;

   /***** Get parameters from form *****/
   /* Get the code of the department */
   if ((Dpt->DptCod = Dpt_GetParamDptCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of department is missing.");

   /* Get parameter with institution code */
   Dpt->InsCod = Ins_GetAndCheckParamOtherInsCod ();

   /***** Update institution in table of departments *****/
   sprintf (Query,"UPDATE departments SET InsCod='%ld' WHERE DptCod='%ld'",
            Dpt->InsCod,Dpt->DptCod);
   DB_QueryUPDATE (Query,"can not update the institution of a department");

   /***** Write message to show the change made *****/
   Lay_ShowAlert (Lay_SUCCESS,Txt_The_institution_of_the_department_has_changed);

   /***** Show the form again *****/
   Dpt_EditDepartments ();
  }

/*****************************************************************************/
/******************* Change the short name of a department *******************/
/*****************************************************************************/

void Dpt_RenameDepartShort (void)
  {
   Dpt_RenameDepartment (Cns_SHRT_NAME);
  }

/*****************************************************************************/
/******************* Change the full name of a department ********************/
/*****************************************************************************/

void Dpt_RenameDepartFull (void)
  {
   Dpt_RenameDepartment (Cns_FULL_NAME);
  }

/*****************************************************************************/
/************************ Change the name of a degree ************************/
/*****************************************************************************/

static void Dpt_RenameDepartment (Cns_ShrtOrFullName_t ShrtOrFullName)
  {
   extern const char *Txt_You_can_not_leave_the_name_of_the_department_X_empty;
   extern const char *Txt_The_department_X_already_exists;
   extern const char *Txt_The_department_X_has_been_renamed_as_Y;
   extern const char *Txt_The_name_of_the_department_X_has_not_changed;
   struct Department *Dpt;
   const char *ParamName = NULL;	// Initialized to avoid warning
   const char *FieldName = NULL;	// Initialized to avoid warning
   size_t MaxBytes = 0;			// Initialized to avoid warning
   char *CurrentDptName = NULL;		// Initialized to avoid warning
   char NewDptName[Hie_MAX_BYTES_FULL_NAME + 1];

   Dpt = &Gbl.Dpts.EditingDpt;
   switch (ShrtOrFullName)
     {
      case Cns_SHRT_NAME:
         ParamName = "ShortName";
         FieldName = "ShortName";
         MaxBytes = Hie_MAX_BYTES_SHRT_NAME;
         CurrentDptName = Dpt->ShrtName;
         break;
      case Cns_FULL_NAME:
         ParamName = "FullName";
         FieldName = "FullName";
         MaxBytes = Hie_MAX_BYTES_FULL_NAME;
         CurrentDptName = Dpt->FullName;
         break;
     }

   /***** Get parameters from form *****/
   /* Get the code of the department */
   if ((Dpt->DptCod = Dpt_GetParamDptCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of department is missing.");

   /* Get the new name for the department */
   Par_GetParToText (ParamName,NewDptName,MaxBytes);

   /***** Get from the database the old names of the department *****/
   Dpt_GetDataOfDepartmentByCod (Dpt);

   /***** Check if new name is empty *****/
   if (!NewDptName[0])
     {
      sprintf (Gbl.Message,Txt_You_can_not_leave_the_name_of_the_department_X_empty,
               CurrentDptName);
      Lay_ShowAlert (Lay_WARNING,Gbl.Message);
     }
   else
     {
      /***** Check if old and new names are the same (this happens when user press enter with no changes in the form) *****/
      if (strcmp (CurrentDptName,NewDptName))	// Different names
        {
         /***** If degree was in database... *****/
         if (Dpt_CheckIfDepartmentNameExists (ParamName,NewDptName,Dpt->DptCod))
           {
            sprintf (Gbl.Message,Txt_The_department_X_already_exists,
                     NewDptName);
            Lay_ShowAlert (Lay_WARNING,Gbl.Message);
           }
         else
           {
            /* Update the table changing old name by new name */
            Dpt_UpdateDegNameDB (Dpt->DptCod,FieldName,NewDptName);

            /* Write message to show the change made */
            sprintf (Gbl.Message,Txt_The_department_X_has_been_renamed_as_Y,
                     CurrentDptName,NewDptName);
            Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
           }
        }
      else	// The same name
        {
         sprintf (Gbl.Message,Txt_The_name_of_the_department_X_has_not_changed,
                  CurrentDptName);
         Lay_ShowAlert (Lay_INFO,Gbl.Message);
        }
     }

   /***** Show the form again *****/
   Str_Copy (CurrentDptName,NewDptName,
             MaxBytes);
   Dpt_EditDepartments ();
  }

/*****************************************************************************/
/******************* Check if the name of department exists ******************/
/*****************************************************************************/

static bool Dpt_CheckIfDepartmentNameExists (const char *FieldName,const char *Name,long DptCod)
  {
   char Query[256 + Hie_MAX_BYTES_FULL_NAME];

   /***** Get number of departments with a name from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM departments"
	          " WHERE %s='%s' AND DptCod<>'%ld'",
            FieldName,Name,DptCod);
   return (DB_QueryCOUNT (Query,"can not check if the name of a department already existed") != 0);
  }

/*****************************************************************************/
/************* Update department name in table of departments ****************/
/*****************************************************************************/

static void Dpt_UpdateDegNameDB (long DptCod,const char *FieldName,const char *NewDptName)
  {
   char Query[128 + Hie_MAX_BYTES_FULL_NAME];

   /***** Update department changing old name by new name *****/
   sprintf (Query,"UPDATE departments SET %s='%s' WHERE DptCod='%ld'",
	    FieldName,NewDptName,DptCod);
   DB_QueryUPDATE (Query,"can not update the name of a department");
  }

/******************************************************************************/
/*********************** Change the URL of a department *********************/
/*****************************************************************************/

void Dpt_ChangeDptWWW (void)
  {
   extern const char *Txt_The_new_web_address_is_X;
   extern const char *Txt_You_can_not_leave_the_web_address_empty;
   struct Department *Dpt;
   char Query[256 + Cns_MAX_BYTES_WWW];
   char NewWWW[Cns_MAX_BYTES_WWW + 1];

   Dpt = &Gbl.Dpts.EditingDpt;

   /***** Get parameters from form *****/
   /* Get the code of the department */
   if ((Dpt->DptCod = Dpt_GetParamDptCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of department is missing.");

   /* Get the new WWW for the department */
   Par_GetParToText ("WWW",NewWWW,Cns_MAX_BYTES_WWW);

   /***** Check if new WWW is empty *****/
   if (NewWWW[0])
     {
      /* Update the table changing old WWW by new WWW */
      sprintf (Query,"UPDATE departments SET WWW='%s' WHERE DptCod='%ld'",
               NewWWW,Dpt->DptCod);
      DB_QueryUPDATE (Query,"can not update the web of a department");

      /***** Write message to show the change made *****/
      sprintf (Gbl.Message,Txt_The_new_web_address_is_X,
               NewWWW);
      Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
     }
   else
     {
      sprintf (Gbl.Message,"%s",Txt_You_can_not_leave_the_web_address_empty);
      Lay_ShowAlert (Lay_WARNING,Gbl.Message);
     }

   /***** Show the form again *****/
   Str_Copy (Dpt->WWW,NewWWW,
             Cns_MAX_BYTES_WWW);
   Dpt_EditDepartments ();
  }

/*****************************************************************************/
/******************* Put a form to create a new department *******************/
/*****************************************************************************/

static void Dpt_PutFormToCreateDepartment (void)
  {
   extern const char *Hlp_INSTITUTION_Departments_edit;
   extern const char *Txt_New_department;
   extern const char *Txt_Institution;
   extern const char *Txt_Short_name;
   extern const char *Txt_Full_name;
   extern const char *Txt_WWW;
   extern const char *Txt_Another_institution;
   extern const char *Txt_Create_department;
   struct Department *Dpt;
   unsigned NumIns;

   Dpt = &Gbl.Dpts.EditingDpt;

   /***** Start form *****/
   Act_FormStart (ActNewDpt);

   /***** Start of frame *****/
   Lay_StartRoundFrameTable (NULL,Txt_New_department,
                             NULL,Hlp_INSTITUTION_Departments_edit,2);

   /***** Write heading *****/
   fprintf (Gbl.F.Out,"<tr>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "</tr>",
            Txt_Institution,
            Txt_Short_name,
            Txt_Full_name,
            Txt_WWW);

   /***** Institution *****/
   fprintf (Gbl.F.Out,"<tr>"
                      "<td class=\"CENTER_MIDDLE\">"
                      "<select name=\"OthInsCod\" style=\"width:62px;\">"
                      "<option value=\"0\"");
   if (Dpt->InsCod == 0)
      fprintf (Gbl.F.Out," selected=\"selected\"");
   fprintf (Gbl.F.Out,">%s</option>",Txt_Another_institution);
   for (NumIns = 0;
	NumIns < Gbl.Inss.Num;
	NumIns++)
      fprintf (Gbl.F.Out,"<option value=\"%ld\"%s>%s</option>",
               Gbl.Inss.Lst[NumIns].InsCod,
               Gbl.Inss.Lst[NumIns].InsCod == Dpt->InsCod ? " selected=\"selected\"" :
        	                                            "",
               Gbl.Inss.Lst[NumIns].ShrtName);
   fprintf (Gbl.F.Out,"</select>"
	              "</td>");

   /***** Department short name *****/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
                      "<input type=\"text\" name=\"ShortName\""
                      " maxlength=\"%u\" value=\"%s\""
                      " class=\"INPUT_SHORT_NAME\""
                      " required=\"required\" />"
                      "</td>",
            Hie_MAX_CHARS_SHRT_NAME,Dpt->ShrtName);

   /***** Department full name *****/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
                      "<input type=\"text\" name=\"FullName\""
                      " maxlength=\"%u\" value=\"%s\""
                      " class=\"INPUT_FULL_NAME\""
                      " required=\"required\" />"
                      "</td>",
            Hie_MAX_CHARS_FULL_NAME,Dpt->FullName);

   /***** Department WWW *****/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
                      "<input type=\"url\" name=\"WWW\""
                      " maxlength=\"%u\" value=\"%s\""
                      " class=\"INPUT_WWW\""
                      " required=\"required\" />"
                      "</td>"
                      "</tr>",
            Cns_MAX_CHARS_WWW,Dpt->WWW);

   /***** Send button and end of frame *****/
   Lay_EndRoundFrameTableWithButton (Lay_CREATE_BUTTON,Txt_Create_department);

   /***** End of form *****/
   Act_FormEnd ();
  }

/*****************************************************************************/
/******************** Write header with fields of a degree *******************/
/*****************************************************************************/

static void Dpt_PutHeadDepartments (void)
  {
   extern const char *Txt_Code;
   extern const char *Txt_Institution;
   extern const char *Txt_Short_name;
   extern const char *Txt_Full_name;
   extern const char *Txt_WWW;
   extern const char *Txt_Teachers_ABBREVIATION;

   fprintf (Gbl.F.Out,"<tr>"
                      "<th class=\"CENTER_MIDDLE\">"
                      "</th>"
                      "<th class=\"RIGHT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"RIGHT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "</tr>",
            Txt_Code,
            Txt_Institution,
            Txt_Short_name,
            Txt_Full_name,
            Txt_WWW,
            Txt_Teachers_ABBREVIATION);
  }

/*****************************************************************************/
/***************** Receive form to create a new department *******************/
/*****************************************************************************/

void Dpt_RecFormNewDpt (void)
  {
   extern const char *Txt_The_department_X_already_exists;
   extern const char *Txt_You_must_specify_the_web_address_of_the_new_department;
   extern const char *Txt_You_must_specify_the_short_name_and_the_full_name_of_the_new_department;
   struct Department *Dpt;

   Dpt = &Gbl.Dpts.EditingDpt;

   /***** Get parameters from form *****/
   /* Get institution */
   Dpt->InsCod = Ins_GetAndCheckParamOtherInsCod ();

   /* Get department short name */
   Par_GetParToText ("ShortName",Dpt->ShrtName,Hie_MAX_BYTES_SHRT_NAME);

   /* Get department full name */
   Par_GetParToText ("FullName",Dpt->FullName,Hie_MAX_BYTES_FULL_NAME);

   /* Get department WWW */
   Par_GetParToText ("WWW",Dpt->WWW,Cns_MAX_BYTES_WWW);

   if (Dpt->ShrtName[0] && Dpt->FullName[0])	// If there's a department name
     {
      if (Dpt->WWW[0])
        {
         /***** If name of department was in database... *****/
         if (Dpt_CheckIfDepartmentNameExists ("ShortName",Dpt->ShrtName,-1L))
           {
            sprintf (Gbl.Message,Txt_The_department_X_already_exists,
                     Dpt->ShrtName);
            Lay_ShowAlert (Lay_WARNING,Gbl.Message);
           }
         else if (Dpt_CheckIfDepartmentNameExists ("FullName",Dpt->FullName,-1L))
           {
            sprintf (Gbl.Message,Txt_The_department_X_already_exists,
                     Dpt->FullName);
            Lay_ShowAlert (Lay_WARNING,Gbl.Message);
           }
         else	// Add new department to database
            Dpt_CreateDepartment (Dpt);
        }
      else	// If there is not a web
        {
         sprintf (Gbl.Message,"%s",Txt_You_must_specify_the_web_address_of_the_new_department);
         Lay_ShowAlert (Lay_WARNING,Gbl.Message);
        }
     }
   else	// If there is not a department name
     {
      sprintf (Gbl.Message,"%s",Txt_You_must_specify_the_short_name_and_the_full_name_of_the_new_department);
      Lay_ShowAlert (Lay_WARNING,Gbl.Message);
     }

   /***** Show the form again *****/
   Dpt_EditDepartments ();
  }

/*****************************************************************************/
/************************** Create a new department **************************/
/*****************************************************************************/

static void Dpt_CreateDepartment (struct Department *Dpt)
  {
   extern const char *Txt_Created_new_department_X;
   char Query[1024];

   /***** Create a new department *****/
   sprintf (Query,"INSERT INTO departments (InsCod,ShortName,FullName,WWW)"
                  " VALUES ('%ld','%s','%s','%s')",
            Dpt->InsCod,Dpt->ShrtName,Dpt->FullName,Dpt->WWW);
   DB_QueryINSERT (Query,"can not create a new department");

   /***** Write success message *****/
   sprintf (Gbl.Message,Txt_Created_new_department_X,
            Dpt->FullName);
   Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
  }

/*****************************************************************************/
/************************** Get number of departments ************************/
/*****************************************************************************/

unsigned Dpt_GetTotalNumberOfDepartments (void)
  {
   char Query[512];

   /***** Get number of departments from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM departments");
   return (unsigned) DB_QueryCOUNT (Query,"can not get number of departments");
  }

/*****************************************************************************/
/**************** Get number of departments in a institution *****************/
/*****************************************************************************/

unsigned Dpt_GetNumberOfDepartmentsInInstitution (long InsCod)
  {
   char Query[512];

   /***** Get departments in an institution from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM departments"
                  " WHERE InsCod='%ld'",InsCod);
   return (unsigned) DB_QueryCOUNT (Query,"can not get number of departments in an institution");
  }

/*****************************************************************************/
/*********************** Put selector for department *************************/
/*****************************************************************************/

void Dpt_WriteSelectorDepartment (long InsCod)
  {
   extern const char *Txt_Any_department;
   unsigned NumDpt;

   /***** Form to select department *****/
   /* Get list of departments */
   Dpt_GetListDepartments (InsCod);

   /* List departments */
   fprintf (Gbl.F.Out,"<select id=\"DptCod\" name=\"DptCod\""
	              " style=\"width:375px;\""
	              " onchange=\"document.getElementById('%s').submit();\">",
            Gbl.Form.Id);

   fprintf (Gbl.F.Out,"<option value=\"-1\"");
   if (Gbl.Stat.DptCod == -1L)
      fprintf (Gbl.F.Out," selected=\"selected\"");
   fprintf (Gbl.F.Out,">%s</option>",Txt_Any_department);

   for (NumDpt = 0;
	NumDpt < Gbl.Dpts.Num;
	NumDpt++)
     {
      fprintf (Gbl.F.Out,"<option value=\"%ld\"",Gbl.Dpts.Lst[NumDpt].DptCod);
      if (Gbl.Dpts.Lst[NumDpt].DptCod == Gbl.Stat.DptCod)
         fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out,">%s</option>",Gbl.Dpts.Lst[NumDpt].FullName);
     }

   fprintf (Gbl.F.Out,"</select>");

   /* Free list of departments */
   Dpt_FreeListDepartments ();
  }

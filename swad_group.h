// swad_group.h: groups

#ifndef _SWAD_GRP
#define _SWAD_GRP
/*
    SWAD (Shared Workspace At a Distance in Spanish),
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

#include "swad_info.h"
#include "swad_user.h"

/*****************************************************************************/
/***************************** Public constants ******************************/
/*****************************************************************************/

#define Grp_MAX_LENGTH_GROUP_TYPE_NAME	(256 - 1)
#define Grp_MAX_LENGTH_GROUP_NAME	(256 - 1)

#define Grp_MAX_STUDENTS_IN_A_GROUP	10000	// If max of students in a group is greater than this, it is considered infinite

/*****************************************************************************/
/******************************* Public types ********************************/
/*****************************************************************************/

typedef enum
  {
   Grp_ONLY_GROUP_TYPES_WITH_GROUPS,
   Grp_ALL_GROUP_TYPES,
  } Grp_WhichGroupTypes_t;

// Related with groups
struct GroupData
  {
   long GrpCod;
   long GrpTypCod;
   long CrsCod;
   char GrpTypName[Grp_MAX_LENGTH_GROUP_TYPE_NAME + 1];
   char GrpName[Grp_MAX_LENGTH_GROUP_NAME + 1];
   unsigned MaxStudents;
   int  Vacant;
   bool Open;					// Group is open?
   bool FileZones;				// Group has file zones?
   bool MultipleEnrollment;
  };
struct Group
  {
   long GrpCod;					// Code of group
   char GrpName[Grp_MAX_LENGTH_GROUP_NAME + 1];	// Name of group
   unsigned MaxStudents;
   unsigned NumStudents;			// Number of students in the group
   bool Open;					// Group is open?
   bool FileZones;				// Group has file zones?
   bool ShowFileZone;				// Show file zone of this group?
  };
struct GroupType
  {
   long GrpTypCod;					// Code of type of group
   char GrpTypName[Grp_MAX_LENGTH_GROUP_TYPE_NAME + 1];	// Name of type of group
   bool MandatoryEnrollment;				// Enrollment is mandatory?
   bool MultipleEnrollment;				// Enrollment is multiple?
   bool MustBeOpened;					// Groups must be opened?
   time_t OpenTimeUTC;					// Open groups automatically in this date-time. If == 0, don't open.
   unsigned NumGrps;					// Number of groups of this type
   struct Group *LstGrps;				// List of groups of this type
  };
struct GroupTypes
  {
   struct GroupType *LstGrpTypes;		// List of types of group
   unsigned Num;				// Number of types of group
   unsigned NumGrpsTotal;			// Number of groups of any type
   int NestedCalls;				// Number of nested calls to the function that allocates memory for this list
  };
struct ListCodGrps
  {
   long *GrpCod;
   unsigned NumGrps;
   int NestedCalls;				// Number of nested calls to the function that allocates memory for this list
  };
struct ListGrpsAlreadySelec
  {
   long GrpTypCod;
   bool AlreadySelected;
  };
typedef enum
  {
   Grp_ONLY_MY_GROUPS,
   Grp_ALL_GROUPS,
  } Grp_WhichGroups_t;
typedef enum
  {
   Grp_ASSIGNMENT,
   Grp_ATT_EVENT,
   Grp_SURVEY,
  } Grp_AsgOrSvy_t;

/*****************************************************************************/
/****************************** Public prototypes ****************************/
/*****************************************************************************/

void Grp_WriteNamesOfSelectedGrps (void);
void Grp_ReqEditGroups (void);

void Grp_ShowFormToSelectSeveralGroups (Act_Action_t NextAction);
void Grp_PutParamsCodGrps (void);
void Grp_GetParCodsSeveralGrpsToShowUsrs (void);
void Grp_GetParCodsSeveralGrpsToEditAsgAttOrSvy (void);
void Grp_FreeListCodSelectedGrps (void);
void Grp_ChangeMyGrpsAndShowChanges (void);
void Grp_ChangeMyGrps (void);
void Grp_ChangeOtherUsrGrps (void);
bool Grp_ChangeMyGrpsAtomically (struct ListCodGrps *LstGrpsIWant);
bool Grp_ChangeGrpsOtherUsrAtomically (struct ListCodGrps *LstGrpsUsrWants);
bool Grp_CheckIfSelectionGrpsIsValid (struct ListCodGrps *LstGrps);
void Grp_RegisterUsrIntoGroups (struct UsrData *UsrDat,struct ListCodGrps *LstGrps);
unsigned Grp_RemoveUsrFromGroups (struct UsrData *UsrDat,struct ListCodGrps *LstGrps);
void Grp_RemUsrFromAllGrpsInCrs (struct UsrData *UsrDat,struct Course *Crs,Cns_QuietOrVerbose_t QuietOrVerbose);
void Grp_RemUsrFromAllGrps (struct UsrData *UsrDat,Cns_QuietOrVerbose_t QuietOrVerbose);
void Grp_ListGrpsToEditAsgAttOrSvy (struct GroupType *GrpTyp,long Cod,Grp_AsgOrSvy_t Grp_AsgOrSvy);

void Grp_ReqRegisterInGrps (void);
void Grp_ShowLstGrpsToChgMyGrps (bool ShowWarningsToStudents);
void Grp_ShowLstGrpsToChgOtherUsrsGrps (long UsrCod);

void Grp_GetListGrpTypesInThisCrs (Grp_WhichGroupTypes_t WhichGroupTypes);
void Grp_FreeListGrpTypesAndGrps (void);
void Grp_OpenGroupsAutomatically (void);
void Grp_GetListGrpTypesAndGrpsInThisCrs (Grp_WhichGroupTypes_t WhichGroupTypes);
unsigned Grp_CountNumGrpsInCurrentCrs (void);
unsigned long Grp_GetGrpsOfType (long GrpTypCod,MYSQL_RES **mysql_res);
void Grp_GetDataOfGroupByCod (struct GroupData *GrpDat);
bool Grp_CheckIfGroupExists (long GrpCod);
bool Grp_CheckIfGroupBelongsToCourse (long GrpCod,long CrsCod);
unsigned Grp_CountNumStdsInGrp (long GrpCod);
bool Grp_GetIfIBelongToGrp (long GrpCod);
unsigned Grp_NumGrpTypesMandatIDontBelong (void);
void Grp_GetLstCodGrpsWithFileZonesIBelong (struct ListCodGrps *LstGrps);
void Grp_GetNamesGrpsStdBelongsTo (long GrpTypCod,long UsrCod,char *GrpNames);
void Grp_RecFormNewGrpTyp (void);
void Grp_RecFormNewGrp (void);
void Grp_ReqRemGroupType (void);
void Grp_ReqRemGroup (void);
void Grp_RemoveGroupType (void);
void Grp_RemoveGroup (void);
void Grp_OpenGroup (void);
void Grp_CloseGroup (void);
void Grp_EnableFileZonesGrp (void);
void Grp_DisableFileZonesGrp (void);
void Grp_ChangeGroupType (void);
void Grp_ChangeMandatGrpTyp (void);
void Grp_ChangeMultiGrpTyp (void);
void Grp_ChangeOpenTimeGrpTyp (void);
void Grp_ChangeMaxStdsGrp (void);
unsigned Grp_ConvertToNumMaxStdsGrp (const char *StrMaxStudents);
void Grp_RenameGroupType (void);
void Grp_RenameGroup (void);
void Grp_PutParamGrpCod (long GrpCod);
void Grp_GetLstCodsGrpWanted (struct ListCodGrps *LstGrpsWanted);
void Grp_FreeListCodGrp (struct ListCodGrps *LstGrps);
void Grp_PutParamAllGroups (void);

void Grp_PutParamWhichGrps (void);
void Grp_PutParamWhichGrpsOnlyMyGrps (void);
void Grp_PutParamWhichGrpsAllGrps (void);
void Grp_ShowFormToSelWhichGrps (Act_Action_t Action,void (*FuncParams) ());
void Grp_GetParamWhichGrps (void);

#endif

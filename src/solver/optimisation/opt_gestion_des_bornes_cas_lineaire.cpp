/*
** Copyright 2007-2018 RTE
** Authors: Antares_Simulator Team
**
** This file is part of Antares_Simulator.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** There are special exceptions to the terms and conditions of the
** license as they are applied to this software. View the full text of
** the exceptions in file COPYING.txt in the directory of this software
** distribution
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Antares_Simulator. If not, see <http://www.gnu.org/licenses/>.
**
** SPDX-License-Identifier: licenceRef-GPL3_WITH_RTE-Exceptions
*/

#include "opt_structure_probleme_a_resoudre.h"

#include "../simulation/simulation.h"
#include "../simulation/sim_structure_donnees.h"
#include "../simulation/sim_structure_probleme_economique.h"
#include "../simulation/sim_structure_probleme_adequation.h"
#include "../simulation/sim_extern_variables_globales.h"

#include "opt_fonctions.h"
#include <math.h>
#include <yuni/core/math.h>
#include <limits.h>

#include "../ext/Sirius_Solver/simplexe/spx_constantes_externes.h"

#define EPSILON_DEFAILLANCE   1e-3

using namespace Yuni;





void OPT_MaxDesPmaxHydrauliques( PROBLEME_HEBDO * ProblemeHebdo )
{
int Pays; int PdtHebdo; double PmaxHyd;
double * ContrainteDePmaxHydrauliqueHoraire;

for ( Pays = 0 ; Pays < ProblemeHebdo->NombreDePays ; Pays++ ) {
  ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->MaxDesPmaxHydrauliques = 0.0; 
  ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->MaxDesPmaxHydrauliquesRef = 0.0;
	if ( ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->PresenceDHydrauliqueModulable != OUI_ANTARES ) continue;
  ContrainteDePmaxHydrauliqueHoraire = ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->ContrainteDePmaxHydrauliqueHoraire;
	PmaxHyd = -1;
  for ( PdtHebdo = 0 ; PdtHebdo < ProblemeHebdo->NombreDePasDeTemps ; PdtHebdo++ ) {
	  if ( ContrainteDePmaxHydrauliqueHoraire[PdtHebdo] > PmaxHyd ) PmaxHyd = ContrainteDePmaxHydrauliqueHoraire[PdtHebdo];
  }
  ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->MaxDesPmaxHydrauliques = PmaxHyd;
  ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->MaxDesPmaxHydrauliquesRef = PmaxHyd;
}
			 
return;
}




static double OPT_SommeDesPmaxThermiques( PROBLEME_HEBDO * ProblemeHebdo, int Pays, int PdtHebdo )
{
int Index; double SommeDesPmaxThermiques;
PDISP_ET_COUTS_HORAIRES_PAR_PALIER ** PuissanceDisponibleEtCout;
PALIERS_THERMIQUES                 *  PaliersThermiquesDuPays;

SommeDesPmaxThermiques    = 0.0;
PaliersThermiquesDuPays   = ProblemeHebdo->PaliersThermiquesDuPays[Pays];
PuissanceDisponibleEtCout = PaliersThermiquesDuPays->PuissanceDisponibleEtCout;

for (Index = 0; Index < PaliersThermiquesDuPays->NombreDePaliersThermiques; Index++) {
	SommeDesPmaxThermiques += PuissanceDisponibleEtCout[Index]->PuissanceDisponibleDuPalierThermique[PdtHebdo];
}

return( SommeDesPmaxThermiques );
}








double OPT_SommeDesPminThermiques( PROBLEME_HEBDO * ProblemeHebdo, int Pays, int PdtHebdo )
{
int Index; double SommeDesPminThermiques;
PDISP_ET_COUTS_HORAIRES_PAR_PALIER ** PuissanceDisponibleEtCout;
PALIERS_THERMIQUES                 *  PaliersThermiquesDuPays;

SommeDesPminThermiques    = 0.0;
PaliersThermiquesDuPays   = ProblemeHebdo->PaliersThermiquesDuPays[Pays];
PuissanceDisponibleEtCout = PaliersThermiquesDuPays->PuissanceDisponibleEtCout;

for (Index = 0; Index < PaliersThermiquesDuPays->NombreDePaliersThermiques; Index++) {
	SommeDesPminThermiques +=	PuissanceDisponibleEtCout[Index]->PuissanceMinDuPalierThermique[PdtHebdo];
}

return( SommeDesPminThermiques );
}




void OPT_InitialiserLesBornesDesVariablesDuProblemeLineaire( PROBLEME_HEBDO * ProblemeHebdo,
                                                             const int PremierPdtDeLIntervalle,
                                                         	   const int DernierPdtDeLIntervalle,
																														 const int NumeroDeLIntervalle )
{
int PdtHebdo; int PdtJour; int Interco; int Pays; int Palier; int Var; int Index; double PmxTh;
double * AdresseDuResultat ; int maxThermalPlant; double Sup; double Wj; double C ;
double ** AdresseOuPlacerLaValeurDesVariablesOptimisees; double ** AdresseOuPlacerLaValeurDesCoutsReduits;
double * Xmin; double * Xmax; int * TypeDeVariable;

VALEURS_DE_NTC_ET_RESISTANCES * ValeursDeNTC           ; CORRESPONDANCES_DES_VARIABLES      * CorrespondanceVarNativesVarOptim;
PALIERS_THERMIQUES            * PaliersThermiquesDuPays; PDISP_ET_COUTS_HORAIRES_PAR_PALIER * PuissanceDisponibleEtCout       ; 
CONSOMMATIONS_ABATTUES * ConsommationsAbattues         ; COUTS_DE_TRANSPORT                 * CoutDeTransport                 ;
PROBLEME_ANTARES_A_RESOUDRE * ProblemeAResoudre                ;
 
ProblemeAResoudre = ProblemeHebdo->ProblemeAResoudre;

AdresseOuPlacerLaValeurDesVariablesOptimisees = ProblemeAResoudre->AdresseOuPlacerLaValeurDesVariablesOptimisees;
AdresseOuPlacerLaValeurDesCoutsReduits        = ProblemeAResoudre->AdresseOuPlacerLaValeurDesCoutsReduits;
Xmin = ProblemeAResoudre->Xmin;
Xmax = ProblemeAResoudre->Xmax;
TypeDeVariable = ProblemeAResoudre->TypeDeVariable;

for ( Var = 0 ; Var < ProblemeAResoudre->NombreDeVariables ; Var++ ) {
	AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = NULL;
	AdresseOuPlacerLaValeurDesCoutsReduits       [Var] = NULL;
}

for ( PdtHebdo = PremierPdtDeLIntervalle , PdtJour  = 0 ; PdtHebdo < DernierPdtDeLIntervalle ; PdtHebdo++ , PdtJour++ ) {

	CorrespondanceVarNativesVarOptim = ProblemeHebdo->CorrespondanceVarNativesVarOptim[PdtJour];
	ValeursDeNTC                     = ProblemeHebdo->ValeursDeNTC[PdtHebdo];

	for ( Interco = 0 ; Interco < ProblemeHebdo->NombreDInterconnexions ; Interco++ ) {
		Var = CorrespondanceVarNativesVarOptim->NumeroDeVariableDeLInterconnexion[Interco];
		


		Xmax[Var] =  ValeursDeNTC->ValeurDeNTCOrigineVersExtremite[Interco];
		Xmin[Var] = -(ValeursDeNTC->ValeurDeNTCExtremiteVersOrigine[Interco]);

		

		if (Math::Infinite( Xmax[Var] ) == 1) {
			
			if (Math::Infinite( Xmin[Var] ) == -1 )
				TypeDeVariable[Var] = VARIABLE_NON_BORNEE;
			else
				TypeDeVariable[Var] = VARIABLE_BORNEE_INFERIEUREMENT;
		}
		else {
			
			if (Math::Infinite( Xmin[Var] ) == -1 )
				TypeDeVariable[Var] = VARIABLE_BORNEE_SUPERIEUREMENT;
			else
				TypeDeVariable[Var] = VARIABLE_BORNEE_DES_DEUX_COTES;
		}

		
		AdresseDuResultat = &(ProblemeHebdo->VariablesDualesDesContraintesDeNTC[PdtHebdo]->VariableDualeParInterconnexion[Interco]);
		AdresseOuPlacerLaValeurDesCoutsReduits[Var] = AdresseDuResultat;

		AdresseDuResultat = &(ValeursDeNTC->ValeurDuFlux[Interco]);
		AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = AdresseDuResultat;
		
		
		if ( COUT_TRANSPORT == OUI_ANTARES ) {
			CoutDeTransport = ProblemeHebdo->CoutDeTransport[Interco];
			if ( CoutDeTransport->IntercoGereeAvecDesCouts == OUI_ANTARES ) {
				Var = CorrespondanceVarNativesVarOptim->NumeroDeVariableCoutOrigineVersExtremiteDeLInterconnexion[Interco];
				Xmax[Var] = ValeursDeNTC->ValeurDeNTCOrigineVersExtremite[Interco];
        Xmax[Var] += 0.01; 								
				TypeDeVariable[Var] = VARIABLE_BORNEE_DES_DEUX_COTES;
				if (Math::Infinite( Xmax[Var] ) == 1) {
				  TypeDeVariable[Var] = VARIABLE_BORNEE_INFERIEUREMENT;
				}								
				Xmin[Var] = 0.0;
				AdresseOuPlacerLaValeurDesCoutsReduits[Var] = NULL;
				AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = NULL;

				Var = CorrespondanceVarNativesVarOptim->NumeroDeVariableCoutExtremiteVersOrigineDeLInterconnexion[Interco];
				Xmax[Var] = ValeursDeNTC->ValeurDeNTCExtremiteVersOrigine[Interco];
        Xmax[Var] += 0.01; 
				TypeDeVariable[Var] = VARIABLE_BORNEE_DES_DEUX_COTES;
				if (Math::Infinite( Xmax[Var] ) == 1) {
					TypeDeVariable[Var] = VARIABLE_BORNEE_INFERIEUREMENT;
				}				
				Xmin[Var] = 0.0;
				AdresseOuPlacerLaValeurDesCoutsReduits[Var] = NULL;
				AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = NULL;
			}
		}
	} 

	
	ConsommationsAbattues = ProblemeHebdo->ConsommationsAbattues[PdtHebdo];

	for ( Pays = 0 ; Pays < ProblemeHebdo->NombreDePays ; Pays++ ) {
		PaliersThermiquesDuPays = ProblemeHebdo->PaliersThermiquesDuPays[Pays];
		maxThermalPlant         = PaliersThermiquesDuPays->NombreDePaliersThermiques;

		

		for ( Index = 0 ; Index < maxThermalPlant ; Index++ ) {
			Palier = PaliersThermiquesDuPays->NumeroDuPalierDansLEnsembleDesPaliersThermiques[Index];
			Var = CorrespondanceVarNativesVarOptim->NumeroDeVariableDuPalierThermique[Palier];
			PuissanceDisponibleEtCout = PaliersThermiquesDuPays->PuissanceDisponibleEtCout[Index];

			
			Xmin[Var] = PuissanceDisponibleEtCout->PuissanceMinDuPalierThermique[PdtHebdo];
			
			Xmax[Var] = PuissanceDisponibleEtCout->PuissanceDisponibleDuPalierThermique[PdtHebdo];			
			
			AdresseDuResultat = &(ProblemeHebdo->ResultatsHoraires[Pays]->ProductionThermique[PdtHebdo]->ProductionThermiqueDuPalier[Index]);
			AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = AdresseDuResultat;
			
		}

		

		Var = CorrespondanceVarNativesVarOptim->NumeroDeVariablesDeLaProdHyd[Pays];
		ProblemeHebdo->ResultatsHoraires[Pays]->TurbinageHoraire[PdtHebdo] = 0.0;
		if ( Var >= 0 ) {
			Xmin[Var] = 0.0;
			Xmax[Var] = ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->ContrainteDePmaxHydrauliqueHoraire[PdtHebdo];			
			AdresseDuResultat = &(ProblemeHebdo->ResultatsHoraires[Pays]->TurbinageHoraire[PdtHebdo]);
			AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = AdresseDuResultat;
		}

		if ( ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->PresenceDHydrauliqueModulable == OUI_ANTARES ) {
	    if ( ProblemeHebdo->TypeDeLissageHydraulique == LISSAGE_HYDRAULIQUE_SUR_SOMME_DES_VARIATIONS ) {		
		    Var = CorrespondanceVarNativesVarOptim->NumeroDeVariablesVariationHydALaBaisse[Pays];
			  if ( Var >= 0 && Var < ProblemeAResoudre->NombreDeVariables ) {
			    Xmin[Var] = 0.0;
			    Xmax[Var] = LINFINI_ANTARES;
				  AdresseOuPlacerLaValeurDesCoutsReduits[Var] = NULL;
				  AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = NULL;				
				}
		    Var = CorrespondanceVarNativesVarOptim->NumeroDeVariablesVariationHydALaHausse[Pays];
			  if ( Var >= 0 && Var < ProblemeAResoudre->NombreDeVariables ) {
			    Xmin[Var] = 0.0;
			    Xmax[Var] = LINFINI_ANTARES;
				  AdresseOuPlacerLaValeurDesCoutsReduits[Var] = NULL;
				  AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = NULL;				
				}
			}		
	    else if ( ProblemeHebdo->TypeDeLissageHydraulique == LISSAGE_HYDRAULIQUE_SUR_VARIATION_MAX ) {
		    if ( PdtJour == 0 ) {
		      Var = CorrespondanceVarNativesVarOptim->NumeroDeVariablesVariationHydALaBaisse[Pays];
			    if ( Var >= 0 && Var < ProblemeAResoudre->NombreDeVariables ) {
			      Xmin[Var] = 0.0;
            Xmax[Var] = ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->MaxDesPmaxHydrauliques;						
						AdresseOuPlacerLaValeurDesCoutsReduits[Var] = NULL;
				    AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = NULL;				
				  }
		      Var = CorrespondanceVarNativesVarOptim->NumeroDeVariablesVariationHydALaHausse[Pays];
			    if ( Var >= 0 && Var < ProblemeAResoudre->NombreDeVariables ) {
			      Xmin[Var] = 0.0;
            Xmax[Var] = ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->MaxDesPmaxHydrauliques;						
				    AdresseOuPlacerLaValeurDesCoutsReduits[Var] = NULL;
				    AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = NULL;				
				  }		
        }
		  }
		}
				
		
		
		C = ConsommationsAbattues->ConsommationAbattueDuPays[Pays];
		
		if ( ProblemeHebdo->YaDeLaReserveJmoins1 == OUI_ANTARES && ProblemeAResoudre->NumeroDOptimisation == PREMIERE_OPTIMISATION ) {
			C += ProblemeHebdo->ReserveJMoins1[Pays]->ReserveHoraireJMoins1[PdtHebdo];
		}

		
		{
			Var = CorrespondanceVarNativesVarOptim->NumeroDeVariableDefaillancePositive[Pays];
			Xmin[Var] = 0.0;

			
			
			
			
			
						
			
			  

			  
			

			

			if (C >= 0.) Xmax[Var] = C + 1e-5;
			else         Xmax[Var] = 0.;			
			
			
			
			if ( ProblemeHebdo->BorneDeLaDefaillancePositive == EGOISTE && 0  ) {
				PmxTh =  OPT_SommeDesPmaxThermiques( ProblemeHebdo, Pays, PdtHebdo );
				if ( PmxTh > Xmax[Var] ) {
					
					Xmax[Var] = 0.0;
				}
				else {
					
					Xmax[Var] -= PmxTh;
				}
			}
			
			ProblemeHebdo->ResultatsHoraires[Pays]->ValeursHorairesDeDefaillancePositive[PdtHebdo] = 0.0;
			AdresseDuResultat = &(ProblemeHebdo->ResultatsHoraires[Pays]->ValeursHorairesDeDefaillancePositive[PdtHebdo]);
			AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = AdresseDuResultat;
			
		}

		
		
		
		
		
		
		
		
		
		
		{
			Var = CorrespondanceVarNativesVarOptim->NumeroDeVariableDefaillanceNegative[Pays];
			
			Xmin[Var] = 0.0;

			double v = 0.;
			
			if (ProblemeHebdo->DefaillanceNegativeUtiliserPMinThermique[Pays])
				v += OPT_SommeDesPminThermiques(ProblemeHebdo, Pays, PdtHebdo);


			
			if (ProblemeHebdo->DefaillanceNegativeUtiliserHydro[Pays] && ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->PresenceDHydrauliqueModulable == OUI_ANTARES)
			{
				Sup  = ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->ContrainteDePmaxHydrauliqueHoraire[PdtHebdo];
				Wj   = ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->CntEnergieH2OParIntervalleOptimise[NumeroDeLIntervalle] /
					((double) DernierPdtDeLIntervalle - (double) PremierPdtDeLIntervalle );
				v += (Wj > Sup) ? Wj : Sup;
			}

			
			
			if (C < 0. && ProblemeHebdo->DefaillanceNegativeUtiliserConsoAbattue[Pays])
				v -= C; 

			
			if (C > 0.) v -= C;

			
			
			
			

			



      Xmax[Var] = LINFINI_ANTARES;
			
			
			ProblemeHebdo->ResultatsHoraires[Pays]->ValeursHorairesDeDefaillanceNegative[PdtHebdo] = 0.0;
			AdresseDuResultat = &(ProblemeHebdo->ResultatsHoraires[Pays]->ValeursHorairesDeDefaillanceNegative[PdtHebdo]);
			AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = AdresseDuResultat;
		}

		
		ProblemeHebdo->ResultatsHoraires[Pays]->ValeursHorairesDeDefaillanceEnReserve[PdtHebdo] = 0.0;

		











		
		Var = CorrespondanceVarNativesVarOptim->NumeroDeVariableDefaillanceEnReserve[Pays];
		if ( Var >= 0 ) {
		  Xmin[Var] = 0.0;
			
		  Xmax[Var] = LINFINI_ANTARES;

		  AdresseDuResultat = &(ProblemeHebdo->ResultatsHoraires[Pays]->ValeursHorairesDeDefaillanceEnReserve[PdtHebdo]);
		  AdresseOuPlacerLaValeurDesVariablesOptimisees[Var] = AdresseDuResultat;
		}
		
	}

}

if ( ProblemeHebdo->OptimisationAvecCoutsDeDemarrage == OUI_ANTARES ) {
  OPT_InitialiserLesBornesDesVariablesDuProblemeLineaireCoutsDeDemarrage( ProblemeHebdo, PremierPdtDeLIntervalle, DernierPdtDeLIntervalle );  
}

return;
}



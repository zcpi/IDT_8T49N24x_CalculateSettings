int ceil_func(double x){
	return (int)( x < 0.0 ? x : x+0.9 );
}

int floor_func(double x){
	return (int)( x < 0.0 ? x-0.9 : x );
}

int round_func(double number)
{
    return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}

/************************** Function Definitions *****************************/

/* Get valid Integer output divider values. */
int IDT_8T49N24x_GetIntDivTable(int FOut, int *DivTable, u8 Bypass)
{
	int i;
	int NS1_Options[4] = {1,4,5,6};
	int index;
	int NS2Min = 1;
	int NS2Max = 1;

	int NS2Temp;
	int OutDivTemp;
	u32 VCOTemp;

	int OutDivMin = (int)ceil(IDT_8T49N24X_FVCO_MIN/FOut);
	int OutDivMax = (int)floor(IDT_8T49N24X_FVCO_MAX/FOut);

	int Count = 0;
	int *DivTablePtr = DivTable;

	if (Bypass == TRUE) {
		index = 0;
	} else {
		index = 1;
	}

	for (i = index; i < (sizeof(NS1_Options) / sizeof(int)); i++) {
		if ((NS1_Options[i] == OutDivMin) ||
		    (NS1_Options[i] == OutDivMax)) {
			/* This is for the case where we want to bypass NS2. */
			NS2Min = 0;
			NS2Max = 0;
		}
	}

	if (NS2Min == 1) {
		/* If this test passes, then we know
		 * we're not in the bypass case. */
		/* the last element in the list */
		NS2Min = (int)ceil(OutDivMin / NS1_Options[3] / 2);
		NS2Max = (int)floor(OutDivMax / NS1_Options[index] / 2);
		if (NS2Max == 0) {
			/* Because we're rounding-down for the max,
			 * we may end-up with it being 0,
			 * In which case we need to make it 1
			 * */
			NS2Max = 1;
		}
	}

	NS2Temp = NS2Min;

	while (NS2Temp <= NS2Max) {
		for (i = index; i < (sizeof(NS1_Options) / sizeof(int)); i++) {
			if (NS2Temp == 0) {
				OutDivTemp = NS1_Options[i];
			} else {
				OutDivTemp = NS1_Options[i] * NS2Temp * 2;
			}

			VCOTemp = FOut * OutDivTemp;

			if ((VCOTemp <= IDT_8T49N24X_FVCO_MAX) &&
			    (VCOTemp >= IDT_8T49N24X_FVCO_MIN)) {
				*DivTablePtr = OutDivTemp;
				Count++;
				DivTablePtr++;
			}
		}
		NS2Temp++;
	}

	return Count;
}


static int IDT_8T49N24x_CalculateSettings(int FIn, int FOut,
			IDT_8T49N24x_Settings* RegSettings)
{
	int DivTable[20];
	int DivTableCount;

	int MaxDiv = 0;
	double FVCO;
	int NS1_RegSettings;
	int NS2_RegSettings;

	int NS1Ratio;
	int NS2Ratio;

	double FracDiv;

	double UpperFBDiv;
	int DSMInt_RegSettings;
	int DSMFrac_RegSettings;
	double Ratio;
	int i;

	/* Get the valid integer dividers. */
	DivTableCount = IDT_8T49N24x_GetIntDivTable(FOut, DivTable, FALSE);

	/* Find the highest divider */
	for (i = 0; i < DivTableCount; i++) {
		if (MaxDiv < DivTable[i]) {
			MaxDiv = DivTable[i];
		}
	}

	FVCO = (double)FOut*MaxDiv;

	/* *************************************************
	 * INTEGER DIVIDER: Determine NS1 register setting *
	 * ************************************************/

	/* Only use the divide-by-1 option for
	 * really small divide ratios
	 * Note that this option will never be on
	 * the list for the Q0 - Q3 dividers
	 * */
	if (MaxDiv < 4) {

	}

	/* Make sure we can divide the ratio by 4 in NS1 and
	 * by 1 or an even number in NS2
	 * */
	if ((MaxDiv == 4) ||
	    (MaxDiv % 8 == 0)) {
		NS1_RegSettings = 2; // Divide by 4 register selection
	}

	/* Make sure we can divide the ratio by 5 in NS1 and by 1
	 * or an even number in NS2
	 * */
	if ((MaxDiv == 5) ||
	    (MaxDiv % 10 == 0)) {
		NS1_RegSettings = 0; // Divide by 5 register selection
	}

	/* Make sure we can divide the ratio by 6 in NS1 and by 1
	 * or an even number in NS2
	 * */
	if ((MaxDiv == 6) ||
	    (MaxDiv % 12 == 0)) {
		NS1_RegSettings = 1; // Divide by 6 register setting
	}

	/* *************************************************
	 * INTEGER DIVIDER: Determine NS2 register setting *
	 * ************************************************/

	switch (NS1_RegSettings) {
	case (0):
		NS1Ratio = 5;
		break;

	case (1):
		NS1Ratio = 6;
		break;

	case (2):
		NS1Ratio = 4;
		break;

	case (3): /* This is the bypass (divide-by-1) option. */
		NS1Ratio = 1;
		break;

	default:
		NS1Ratio = 6;
		break;
	}

	NS2Ratio = (int)floor_func(MaxDiv / NS1Ratio);

	NS2_RegSettings = (int)floor_func(NS2Ratio/2);

	/* ********************
	 * FRACTIONAL DIVIDER *
	 * *******************/
	FracDiv = FVCO/FOut;

	u32 N_Q2 = 0;
	u32 NFRAC_Q2 = 0;

	double frac_numerator = round_func(((FracDiv / 2.0) -
					    (int)(FracDiv / 2.0)) *
					   pow(2,28));

	/* This is the case where the fractional portion is 0. */
	/* Due to precision limitations, sometimes fractional
	 * portion of the Effective divider gets rounded to 1.
	 * This checks for that condition
	 * */
	if ((frac_numerator >= 268435456) ||
	    ((FracDiv/2.0) == (int)(FracDiv/2.0))) {
		N_Q2 = (int)round_func(FracDiv / 2.0);
		NFRAC_Q2 = 0;
	} else {
		/* This is the case where the
		 * fractional portion is not 0. */
		N_Q2 = (int)floor_func(FracDiv / 2.0);
		NFRAC_Q2 = (int)frac_numerator;
	}

	/* ***************************************************
	 * Calculate the Upper Loop Feedback divider setting *
	 * **************************************************/

	UpperFBDiv = (double)(FVCO) / (2 * IDT_8T49N24X_XTAL_FREQ);
	DSMInt_RegSettings = (int)floor_func(UpperFBDiv);

	DSMFrac_RegSettings = (int)round_func((UpperFBDiv -
					       floor_func(UpperFBDiv)) *
					      pow(2,21));

	/* ***************************************************
	 * Calculate the Lower Loop Feedback divider and     *
	 * Input Divider                                     *
	 * **************************************************/

	Ratio = FVCO/FIn;

	int M1 = 0;
	int PMin = (int)FIn / IDT_8T49N24X_FPD_MAX;

	/* This M1 divider sets the input PFD
	 * frequency at 128KHz, the set max
	 * */
	//int M1Min = (int)(FVCO / IDT_8T49N24X_FPD_MAX);

	int M1_default = 0;
	int P_default = 0;
	int error_tmp = 999999;
	int error = 99999999;

	int count = 0;

	/* Start from lowest divider and iterate until 0 error is found
	 * or the divider limit is exhausted.
	 * Keep the setting with the lowest error
	 * */
	for (i = PMin; i <= IDT_8T49N24X_P_MAX; i++) {
		M1 = (int)round_func(i*Ratio);
		count++;
		if (M1 < IDT_8T49N24X_M_MAX) {
			error_tmp = (int)(Ratio*1e9 - (M1*1e9 / i));

			if (abs(error_tmp) < error || error_tmp == 0) {
				error = abs(error_tmp);
				M1_default = M1;
				P_default = i;

				if (error_tmp == 0)
					break;
			}
		} else {
			break;
		}
	}

	RegSettings->NS1_Qx = NS1_RegSettings;
	RegSettings->NS2_Qx = NS2_RegSettings;

	RegSettings->N_Qx = N_Q2;
	RegSettings->NFRAC_Qx = NFRAC_Q2;

	RegSettings->DSM_INT = DSMInt_RegSettings;
	RegSettings->DSM_FRAC = DSMFrac_RegSettings;
	RegSettings->M1_x = M1_default;
	RegSettings->PRE_x = P_default;

/*
	RegSettings->NS1_Qx = NS1_RegSettings;
	RegSettings->NS2_Qx = NS2_RegSettings;

	RegSettings->N_Qx = 8;
	RegSettings->NFRAC_Qx = 0;

	RegSettings->DSM_INT = 44;
	RegSettings->DSM_FRAC = 1153434;
	RegSettings->M1_x = 27840;
	RegSettings->PRE_x = 1740;

	xil_printf("NS1 : %0d\n\r", NS1_RegSettings);
	xil_printf("NS2 : %0d\n\r", NS2_RegSettings);
	xil_printf("N_Qx : %0d\n\r", RegSettings->N_Qx);
	xil_printf("NFRAC_Qx : %0d\n\r", RegSettings->NFRAC_Qx);
	xil_printf("DSM_INT : %0d\n\r", RegSettings->DSM_INT);
	xil_printf("DSM_FRAC : %0d\n\r", RegSettings->DSM_FRAC);
	xil_printf("M1_x : %0d\n\r", RegSettings->M1_x);
	xil_printf("PRE_x : %0d\n\r", RegSettings->PRE_x);
*/

	return 1;
}

void main(void) 
{

	IDT_8T49N24x_Settings RegSettings;
	IDT_8T49N24x_GetSettings(0, 270000000, &RegSettings);

}

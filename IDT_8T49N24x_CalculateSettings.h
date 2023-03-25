
/**************************** Type Definitions *******************************/

typedef struct {
	// Integer Output Divider
	u8  NS1_Qx;
	u16 NS2_Qx;

	// Fractional Output Divider
	u32 N_Qx;
	u32 NFRAC_Qx;

	// Upper Loop Feedback Divider
	u16 DSM_INT;
	u32 DSM_FRAC;

	// Lower Loop Dividers
	u32 M1_x;
	u32 PRE_x;

} IDT_8T49N24x_Settings;

static int IDT_8T49N24x_GetIntDivTable(int FOut, int *DivTable, u8 Bypass);
static int IDT_8T49N24x_CalculateSettings(int FIn, int FOut, IDT_8T49N24x_Settings* RegSettings);
int ceil_func(double x);
int floor_func(double x);
int round_func(double number);


/************************** Constant Definitions *****************************/
#define IDT_8T49N24X_REVID 0x0    //!< Device Revision
#define IDT_8T49N24X_DEVID 0x0607 //!< Device ID Code

#define IDT_8T49N24X_XTAL_FREQ 40000000  //!< The frequency of the crystal in Hz

#define IDT_8T49N24X_FVCO_MAX 4000000000 //!< Maximum VCO Operating Frequency in Hz
#define IDT_8T49N24X_FVCO_MIN 3000000000 //!< Minimum VCO Operating Frequency in Hz

#define IDT_8T49N24X_FOUT_MAX 400000000  //!< Maximum Output Frequency in Hz
#define IDT_8T49N24X_FOUT_MIN      8000  //!< Minimum Output Frequency in Hz

#define IDT_8T49N24X_FIN_MAX 875000000  //!< Maximum Input Frequency in Hz
#define IDT_8T49N24X_FIN_MIN      8000  //!< Minimum Input Frequency in Hz

//#define IDT_8T49N24X_FPD_MAX 8000000  //!< Maximum Phase Detector Frequency in Hz
#define IDT_8T49N24X_FPD_MAX 128000  //!< Maximum Phase Detector Frequency in Hz
#define IDT_8T49N24X_FPD_MIN   8000  //!< Minimum Phase Detector Frequency in Hz

#define IDT_8T49N24X_P_MAX pow(2,22)  //!< Maximum P divider value
#define IDT_8T49N24X_M_MAX pow(2,24)  //!< Maximum M multiplier value


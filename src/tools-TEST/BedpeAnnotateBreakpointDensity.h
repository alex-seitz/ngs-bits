#include "TestFramework.h"
#include "Settings.h"

TEST_CLASS(BedpeAnnotateBreakpointDensity_Test)
{
Q_OBJECT
private slots:

	void std_params()
	{
		//test
		EXECUTE("BedpeAnnotateBreakpointDensity", "-in " + TESTDATA("data_in/BedpeAnnotateBreakpointDensity_in1.bedpe") + " -density " + TESTDATA("data_in/BedpeAnnotateBreakpointDensity_density.igv")
				+ " -out out/BedpeAnnotateBreakpointDensity_out1.bedpe");

		COMPARE_FILES("out/BedpeAnnotateBreakpointDensity_out1.bedpe", TESTDATA("data_out/BedpeAnnotateBreakpointDensity_out1.bedpe"));
	}
};

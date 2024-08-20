#include "TestFramework.h"

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <fstream>

int main(int argc, char *argv[])
{
	return TFW::run(argc, argv);
}


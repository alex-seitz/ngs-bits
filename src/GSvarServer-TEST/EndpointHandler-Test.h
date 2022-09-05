#include "TestFramework.h"
#include "ServerController.h"
#include "ServerController.cpp"

TEST_CLASS(EndpointHandler_Test)
{
Q_OBJECT
private slots:
	void test_api_info()
    {
		HttpRequest request;
		request.setMethod(RequestMethod::GET);
		request.setContentType(ContentType::APPLICATION_JSON);
		request.setPrefix("v1");
		request.setPath("info");

		HttpResponse response = ServerController::serveResourceAsset(request);
		QJsonDocument json_doc = QJsonDocument::fromJson(response.getPayload());	;

		IS_TRUE(response.getStatusLine().contains("200"));
		S_EQUAL(json_doc.object()["name"].toString(), ToolBase::applicationName());
		S_EQUAL(json_doc.object()["version"].toString(), ToolBase::version());
		S_EQUAL(json_doc.object()["api_version"].toString(), NGSHelper::serverApiVersion());
    }

	void test_saving_gsvar_file()
	{
		QString url_id = ServerHelper::generateUniqueStr();
		QString file = TESTDATA("data/sample.gsvar");
		QString copy_name = file+"_tmp";
		QFile::copy(file, copy_name);
		QString file_copy = TESTDATA(copy_name.toUtf8());

		IS_FALSE(UrlManager::isInStorageAlready(file_copy));
		UrlManager::addNewUrl(url_id, UrlEntity(QFileInfo(file_copy).fileName(), QFileInfo(file_copy).absolutePath(), file_copy, url_id, QDateTime::currentDateTime()));
		IS_TRUE(UrlManager::isInStorageAlready(file_copy));

		QJsonDocument json_doc = QJsonDocument();
		QJsonArray json_array;
		QJsonObject json_object;
		json_object.insert("variant", "chr1:12062205-12062205 A>G");
		json_object.insert("column", "comment");
		json_object.insert("text", "some text for testing");
		json_array.append(json_object);
		json_doc.setArray(json_array);


		Session cur_session(1, QDateTime::currentDateTime());
		SessionManager::addNewSession("token", cur_session);

		HttpRequest request;
		request.setMethod(RequestMethod::PUT);
		request.setContentType(ContentType::TEXT_HTML);
		request.setPrefix("v1");
		request.setPath("project_file");
		request.addUrlParam("ps_url_id", url_id);
		request.setBody(json_doc.toJson());
		request.addUrlParam("token", "token");

		HttpResponse response = ServerController::saveProjectFile(request);
		IS_TRUE(response.getStatusLine().contains("200"));
		COMPARE_FILES(file_copy, TESTDATA("data/sample_saved_changes.gsvar"));
		QFile::remove(copy_name);
	}

	void test_uploading_file()
	{
		QString url_id = ServerHelper::generateUniqueStr();
		QString file = TESTDATA("data/sample.gsvar");
		QString copy_name = "uploaded_file.txt";
		QByteArray upload_file = TESTDATA("data/to_upload.txt");

		IS_FALSE(UrlManager::isInStorageAlready(upload_file));
		UrlManager::addNewUrl(url_id, UrlEntity(QFileInfo(upload_file).fileName(), QFileInfo(upload_file).absolutePath(), upload_file, url_id, QDateTime::currentDateTime()));
		IS_TRUE(UrlManager::isInStorageAlready(upload_file));

		Session cur_session(1, QDateTime::currentDateTime());
		SessionManager::addNewSession("token", cur_session);

		HttpRequest request;
		request.setMethod(RequestMethod::POST);
		request.setContentType(ContentType::MULTIPART_FORM_DATA);
		request.setPrefix("v1");
		request.setPath("upload");
		request.addUrlParam("token", "token");

		request.setMultipartFileName(copy_name);
		request.setMultipartFileContent(Helper::loadTextFile(upload_file)[0].toUtf8());

		request.addHeader("Accept", "*/*");
		request.addHeader("Content-Type", "multipart/form-data; boundary=------------------------2cb4f6c221043bbe");

		HttpResponse response = ServerController::uploadFile(request);
		IS_TRUE(response.getStatusLine().contains("400"));
		request.addFormDataParam("ps_url_id", url_id);
		response = ServerController::uploadFile(request);
		IS_TRUE(response.getStatusLine().contains("200"));
		QString file_copy = TESTDATA("data/" + copy_name.toUtf8());
		COMPARE_FILES(file_copy, upload_file);
		QFile::remove(file_copy);
	}

	void test_session_info()
	{
		QDateTime login_time = QDateTime::currentDateTime();
		Session cur_session(1, login_time);
		SessionManager::addNewSession("token", cur_session);

		HttpRequest request;
		request.setMethod(RequestMethod::GET);
		request.setContentType(ContentType::APPLICATION_JSON);
		request.setPrefix("v1");
		request.setPath("session");

		HttpResponse response = ServerController::getSessionInfo(request);
		I_EQUAL(response.getStatusCode(), 403);

		request.addUrlParam("token", "token");
		response = ServerController::getSessionInfo(request);
		QJsonDocument json_doc = QJsonDocument::fromJson(response.getPayload());
		QJsonObject  json_object = json_doc.object();

		I_EQUAL(response.getStatusCode(), 200);
		I_EQUAL(json_object.value("user_id").toInt(), 1);
		I_EQUAL(json_object.value("login_time").toInt(), login_time.toSecsSinceEpoch());
		IS_FALSE(json_object.value("is_db_token").toBool());
	}
};

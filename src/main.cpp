#include <cstdlib>
#include <utility>
#include "crow.h"
#include "model/DatabaseConnector.h"
#include "service/record/SystemRecordService.h"
#include "service/record/SystemRecordConverter.h"

int getPort();

std::string getDBConnectionString();

int main() {
  crow::logger::setLogLevel(crow::LogLevel::DEBUG);
  crow::SimpleApp app;
  int PORT = getPort();

  std::string connStr = getDBConnectionString();
  auto dbConnector = std::make_shared<DatabaseConnector>(connStr);
  auto recordRepository = std::make_shared<SystemRecordRepository>(dbConnector);
  SystemRecordService systemRecordService(recordRepository);
  SystemRecordConverter systemRecordConverter;

  CROW_ROUTE(app, "/")([]() {
      return "Hello world!";
  });

  CROW_ROUTE(app, "/records")
    .methods("GET"_method)([&systemRecordService, &systemRecordConverter]() {
        auto records = systemRecordService.getAllRecords();
        crow::json::wvalue json = crow::json::wvalue(crow::json::type::List);

        if (records.empty()) {
          return crow::response(json);
        }
        for (size_t i = 0; i < records.size(); i++) {
          json[i] = systemRecordConverter.systemRecordToJson(records[i]);
        }
        return crow::response{json};
    });

  CROW_ROUTE(app, "/records")
    .methods("POST"_method)([&systemRecordService]() {
        try {
          systemRecordService.createRecord();
          return crow::response(201, "Record created");
        } catch (std::exception &e) {
          return crow::response(500, e.what());
        }
    });

  CROW_ROUTE(app, "/records")
    .methods("DELETE"_method)([&systemRecordService]() {
        systemRecordService.deleteAllRecords();
        return crow::response(200, "All records deleted");
    });

  CROW_ROUTE(app, "/records/<int>")
    .methods("GET"_method)([&systemRecordService, &systemRecordConverter](int id) {
        try {
          SystemRecord record = systemRecordService.getRecordById(id);
          return crow::response{systemRecordConverter.systemRecordToJson(record)};
        } catch (std::exception &e) {
          return crow::response(404, "Record not found");
        }
    });

  CROW_ROUTE(app, "/records/<int>")
    .methods("DELETE"_method)([&systemRecordService](int id) {
        try {
          systemRecordService.deleteRecordById(id);
          return crow::response(200, "Record deleted");
        } catch (std::exception &e) {
          return crow::response(404, "Record not found");
        }
    });

  app.port(PORT).multithreaded().run();

  return 0;
}

int getPort() {
  const char* portEnv = std::getenv("PORT");
  std::string portStr = portEnv ? portEnv : "8080";
  return std::stoi(portStr);
}

std::string getDBConnectionString() {
  std::string db_host = std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "localhost";
  std::string db_port = std::getenv("DB_PORT") ? std::getenv("DB_PORT") : "54321";
  std::string db_user = std::getenv("DB_USER") ? std::getenv("DB_USER") : "devuser";
  std::string db_password = std::getenv("DB_PASSWORD") ? std::getenv("DB_PASSWORD") : "devpw";
  std::string db_name = std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "crowapidb";

  return "host=" + db_host +
         " port=" + db_port +
         " user=" + db_user +
         " password=" + db_password +
         " dbname=" + db_name;
}

/*
Copyright (C) 2014 Declan Ireland <http://github.com/torndeco/extDB>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "db_raw_v3.h"

#include <Poco/Data/MetaColumn.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/Session.h>

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/SQLite/SQLiteException.h>

#include <Poco/Exception.h>

#include <boost/algorithm/string.hpp>


bool DB_RAW_V3::init(AbstractExt *extension,  AbstractExt::DBConnectionInfo *database, const std::string init_str)
{
	extension_ptr = extension;
	database_ptr = database;

	bool status;

	if (database_ptr->type == std::string("MySQL"))
	{
		status = true;
	}
	else if (database_ptr->type == std::string("SQLite"))
	{
		status = true;
	}
	else
	{
		// DATABASE NOT SETUP YET
		#ifdef TESTING
			extension_ptr->console->warn("extDB: DB_RAW_V3: No Database Connection");
		#endif
		extension_ptr->logger->warn("extDB: DB_RAW_V3: No Database Connection");
		status = false;
	}

	if (status)
	{
		if (init_str.empty())
		{
			stringDataTypeCheck = false;
			#ifdef TESTING
				extension_ptr->console->info("extDB: DB_RAW_V3: Initialized: ADD_QUOTES False");
			#endif
			extension_ptr->logger->info("extDB: DB_RAW_V3: Initialized: ADD_QUOTES False");
		}
		else if (boost::iequals(init_str, std::string("ADD_QUOTES")))
		{
			stringDataTypeCheck = true;
			#ifdef TESTING
				extension_ptr->console->info("extDB: DB_RAW_V3: Initialized: ADD_QUOTES True");
			#endif
			extension_ptr->logger->info("extDB: DB_RAW_V3: Initialized: ADD_QUOTES True");
		}
		else 
		{
			status = false;
		}
	}
	return status;
}


bool DB_RAW_V3::callProtocol(std::string input_str, std::string &result, const int unique_id)
{
	try
	{
		#ifdef TESTING
			extension_ptr->console->info("extDB: DB_RAW_V3: Trace: Input: {0}", input_str);
		#endif
		#ifdef DEBUG_LOGGING
			extension_ptr->logger->info("extDB: DB_RAW_V3: Trace: Input: {0}", input_str);
		#endif

		Poco::Data::Session session = extension_ptr->getDBSession_mutexlock(*database_ptr);
		Poco::Data::Statement sql(session);

		sql << input_str;
		sql.execute();
		Poco::Data::RecordSet rs(sql);

		result = "[1,[";
		std::string temp_str;

		std::size_t cols = rs.columnCount();
		if (cols >= 1)
		{
			bool more = rs.moveFirst();
			while (more)
			{
				result += "[";
				for (std::size_t col = 0; col < cols; ++col)
				{
					if (rs[col].isEmpty())
					{
						temp_str.clear();
					}
					else
					{
						temp_str = rs[col].convert<std::string>();
					}
					
					if (stringDataTypeCheck)
						if (rs.columnType(col) == Poco::Data::MetaColumn::FDT_STRING)
						{
							if (temp_str.empty())
							{
								result += ("\"\"");
							}
							else
							{
								result += "\"" + temp_str + "\"";
							}
						}
						else
						{
							if (temp_str.empty())
							{
								result += ("\"\"");
							}
							else
							{
								result += temp_str;
							}
						}
					else
					{
						result += temp_str;
					}
					if (col < (cols - 1))
					{
						result += ",";
					}
				}
				more = rs.moveNext();
				if (more)
				{
					result += "],";
				}
				else
				{
					result += "]";
				}
			}
		}
		result += "]]";
		#ifdef TESTING
			extension_ptr->console->info("extDB: DB_RAW_V3: Trace: Result: {0}", result);
		#endif
		#ifdef DEBUG_LOGGING
			extension_ptr->logger->info("extDB: DB_RAW_V3: Trace: Result: {0}", result);
		#endif
	}
	catch (Poco::InvalidAccessException& e)
	{
		#ifdef TESTING
			extension_ptr->console->error("extDB: DB_RAW_V3: Error InvalidAccessException: {0}", e.displayText());
			extension_ptr->console->error("extDB: DB_RAW_V3: Error InvalidAccessException: SQL: {0}", input_str);
		#endif
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error InvalidAccessException: {0}", e.displayText());
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error InvalidAccessException: SQL: {0}", input_str);
		result = "[0,\"Error DBLocked Exception\"]";
	}
	catch (Poco::Data::NotConnectedException& e)
	{
		#ifdef TESTING
			extension_ptr->console->error("extDB: DB_RAW_V3: Error NotConnectedException: {0}", e.displayText());
			extension_ptr->console->error("extDB: DB_RAW_V3: Error NotConnectedException: SQL: {0}", input_str);
		#endif
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error NotConnectedException: {0}", e.displayText());
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error NotConnectedException: SQL: {0}", input_str);
		result = "[0,\"Error DBLocked Exception\"]";
	}
	catch (Poco::NotImplementedException& e)
	{
		#ifdef TESTING
			extension_ptr->console->error("extDB: DB_RAW_V3: Error NotImplementedException: {0}", e.displayText());
			extension_ptr->console->error("extDB: DB_RAW_V3: Error NotImplementedException: SQL: {0}", input_str);

		#endif
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error NotImplementedException: {0}", e.displayText());
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error NotImplementedException: SQL: {0}", input_str);
		result = "[0,\"Error DBLocked Exception\"]";
	}
	catch (Poco::Data::SQLite::DBLockedException& e)
	{
		#ifdef TESTING
			extension_ptr->console->error("extDB: DB_RAW_V3: Error DBLockedException: {0}", e.displayText());
			extension_ptr->logger->error("extDB: DB_RAW_V3: Error DBLockedException: SQL: {0}", input_str);
		#endif
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error DBLockedException: {0}", e.displayText());
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error DBLockedException: SQL: {0}", input_str);
		result = "[0,\"Error DBLocked Exception\"]";
	}
	catch (Poco::Data::MySQL::ConnectionException& e)
	{
		#ifdef TESTING
			extension_ptr->console->error("extDB: DB_RAW_V3: Error ConnectionException: {0}", e.displayText());
			extension_ptr->logger->error("extDB: DB_RAW_V3: Error ConnectionException: SQL: {0}", input_str);
		#endif
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error ConnectionException: {0}", e.displayText());
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error ConnectionException: SQL: {0}", input_str);
		result = "[0,\"Error Connection Exception\"]";
	}
	catch(Poco::Data::MySQL::StatementException& e)
	{
		#ifdef TESTING
			extension_ptr->console->error("extDB: DB_RAW_V3: Error StatementException: {0}", e.displayText());
			extension_ptr->logger->error("extDB: DB_RAW_V3: Error StatementException: SQL: {0}", input_str);
		#endif
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error StatementException: {0}", e.displayText());
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error StatementException: SQL: {0}", input_str);
		result = "[0,\"Error Statement Exception\"]";
	}
	catch (Poco::Data::DataException& e)
	{
		#ifdef TESTING
			extension_ptr->console->error("extDB: DB_RAW_V3: Error DataException: {0}", e.displayText());
			extension_ptr->logger->error("extDB: DB_RAW_V3: Error DataException: SQL: {0}", input_str);
		#endif
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error DataException: {0}", e.displayText());
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error DataException: SQL: {0}", input_str);
		result = "[0,\"Error Data Exception\"]";
	}
	catch (Poco::Exception& e)
	{
		#ifdef TESTING
			extension_ptr->console->error("extDB: DB_RAW_V3: Error Exception: {0}", e.displayText());
			extension_ptr->console->error("extDB: DB_RAW_V3: Error Exception: SQL: {0}", input_str);
		#endif
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error Exception: {0}", e.displayText());
		extension_ptr->logger->error("extDB: DB_RAW_V3: Error Exception: SQL: {0}", input_str);
		result = "[0,\"Error Exception\"]";
	}
	return true;
}
//
// ODBCStatementImpl.h
//
// $Id: //poco/Main/Data/ODBC/include/Poco/Data/ODBC/ODBCStatementImpl.h#5 $
//
// Library: ODBC
// Package: ODBC
// Module:  ODBCStatementImpl
//
// Definition of the ODBCStatementImpl class.
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef DataConnectors_ODBC_ODBCStatementImpl_INCLUDED
#define DataConnectors_ODBC_ODBCStatementImpl_INCLUDED


#include "Poco/Data/ODBC/ODBC.h"
#include "Poco/Data/ODBC/SessionImpl.h"
#include "Poco/Data/ODBC/Binder.h"
#include "Poco/Data/ODBC/Extractor.h"
#include "Poco/Data/ODBC/Preparation.h"
#include "Poco/Data/ODBC/ODBCColumn.h"
#include "Poco/Data/StatementImpl.h"
#include "Poco/Data/Column.h"
#include "Poco/SharedPtr.h"
#include "Poco/Format.h"
#include <sstream>
#ifdef POCO_OS_FAMILY_WINDOWS
#include <windows.h>
#endif
#include <sqltypes.h>


namespace Poco {
namespace Data {
namespace ODBC {


class ODBC_API ODBCStatementImpl: public Poco::Data::StatementImpl
	/// Implements statement functionality needed for ODBC
{
public:
	typedef std::vector<ODBCColumn*> ColumnPtrVec;

	ODBCStatementImpl(SessionImpl& rSession);
		/// Creates the ODBCStatementImpl.

	~ODBCStatementImpl();
		/// Destroys the ODBCStatementImpl.

protected:
	Poco::UInt32 columnsReturned() const;
		/// Returns number of columns returned by query.

	const MetaColumn& metaColumn(Poco::UInt32 pos) const;
		/// Returns column meta data.

	bool hasNext();
		/// Returns true if a call to next() will return data.

	void next();
		/// Retrieves the next row from the resultset.
		/// Will throw, if the resultset is empty.

	bool canBind() const;
		/// Returns true if a valid statement is set and we can bind.

	void compileImpl();
		/// Compiles the statement, doesn't bind yet

	void bindImpl();
		/// Binds all parameters and executes the statement.

	AbstractExtractor& extractor();
		/// Returns the concrete extractor used by the statement.

	AbstractBinder& binder();
		/// Returns the concrete binder used by the statement.

	std::string nativeSQL();
		/// Returns the SQL string as modified by the driver.

private:
	typedef Poco::Data::AbstractBindingVec Bindings;
	typedef Poco::Data::AbstractExtractionVec Extractions;

	static const std::string INVALID_CURSOR_STATE;

	void clear();
		/// Closes the cursor and resets indicator variables.
	
	void doBind(bool clear = true, bool reset = false);
		/// Binds parameters.

	bool hasData() const;
		/// Returns true if statement returns data.

	bool nextRowReady() const;
		/// Returns true if there is a row fetched but not yet extracted.

	void putData();
		/// Called whenever SQLExecute returns SQL_NEED_DATA. This is expected 
		/// behavior for PB_AT_EXEC binding mode. 

	void getData();

	void fillColumns();
	void checkError(SQLRETURN rc, const std::string& msg="");
	
	bool isStoredProcedure() const;
		/// Returns true if this statement is stored procedure.
		/// Only the ODBC CALL escape sequence is supported.
		/// The function checks whether trimmed statement 
		/// text begins with '{' and ends with '}';

	const SQLHDBC&               _rConnection;
	const StatementHandle        _stmt;
	Poco::SharedPtr<Preparation> _pPreparation;
	Poco::SharedPtr<Binder>      _pBinder;
	Poco::SharedPtr<Extractor>   _pExtractor;
	bool                         _stepCalled;
	int                          _nextResponse;
	ColumnPtrVec                 _columnPtrs;
};


//
// inlines
//
inline AbstractExtractor& ODBCStatementImpl::extractor()
{
	poco_assert_dbg (_pExtractor);
	return *_pExtractor;
}


inline AbstractBinder& ODBCStatementImpl::binder()
{
	poco_assert_dbg (_pBinder);
	return *_pBinder;
}


inline Poco::UInt32 ODBCStatementImpl::columnsReturned() const
{
	poco_assert_dbg (_pPreparation);
	return (Poco::UInt32) _pPreparation->columns();
}


inline bool ODBCStatementImpl::hasData() const
{
	poco_assert_dbg (_pPreparation);
	return (_pPreparation->columns(!isStoredProcedure()) > 0);
}


inline bool ODBCStatementImpl::nextRowReady() const
{
	return (!Utility::isError(_nextResponse));
}


inline const MetaColumn& ODBCStatementImpl::metaColumn(Poco::UInt32 pos) const
{
	std::size_t sz = _columnPtrs.size();

	if (0 == sz || pos > sz - 1)
		throw InvalidAccessException(format("Invalid column number: %u", pos));

	return *_columnPtrs[pos];
}


} } } // namespace Poco::Data::ODBC


#endif // DataConnectors_ODBC_ODBCStatementImpl_INCLUDED

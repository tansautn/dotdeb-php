/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Gustavo Lopes <cataphract@php.net>                          |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_intl.h"
#include "transliterator.h"
#include "transliterator_class.h"
#include "transliterator_methods.h"
#include "intl_data.h"
#include "intl_convert.h"

#include <zend_exceptions.h>

static int create_transliterator( char *str_id, int str_id_len, long direction, zval *object TSRMLS_DC )
{
	Transliterator_object *to;
	UChar	              *ustr_id    = NULL;
	int32_t               ustr_id_len = 0;
	UTransliterator       *utrans;
	UParseError           parse_error   = {0, -1};

	intl_error_reset( NULL TSRMLS_CC );

	if( ( direction != TRANSLITERATOR_FORWARD ) && (direction != TRANSLITERATOR_REVERSE ) )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_create: invalid direction", 0 TSRMLS_CC );
		return FAILURE;
	}

	object_init_ex( object, Transliterator_ce_ptr );
	TRANSLITERATOR_METHOD_FETCH_OBJECT_NO_CHECK; /* fetch zend object from zval "object" into "to" */

	/* Convert transliterator id to UTF-16 */
	intl_convert_utf8_to_utf16( &ustr_id, &ustr_id_len, str_id, str_id_len, TRANSLITERATOR_ERROR_CODE_P( to ) );
	if( U_FAILURE( TRANSLITERATOR_ERROR_CODE( to ) ) )
	{
		intl_error_set_code( NULL, TRANSLITERATOR_ERROR_CODE( to ) TSRMLS_CC );
		intl_error_set_custom_msg( NULL, "String conversion of id to UTF-16 failed", 0 TSRMLS_CC );
		zval_dtor( object );
		return FAILURE;
	}

	/* Open ICU Transliterator. */
	utrans = utrans_openU( ustr_id, ustr_id_len, (UTransDirection ) direction,
		NULL, -1, &parse_error, TRANSLITERATOR_ERROR_CODE_P( to ) );
	if (ustr_id) {
		efree( ustr_id );
	}
	
	if( U_FAILURE( TRANSLITERATOR_ERROR_CODE( to ) ) )
	{
		char *buf = NULL;
		intl_error_set_code( NULL, TRANSLITERATOR_ERROR_CODE( to ) TSRMLS_CC );
		spprintf( &buf, 0, "transliterator_create: unable to open ICU transliterator"
			" with id \"%s\"", str_id );
		if( buf == NULL ) {
			intl_error_set_custom_msg( NULL,
				"transliterator_create: unable to open ICU transliterator", 0 TSRMLS_CC );
		}
		else
		{
			intl_error_set_custom_msg( NULL, buf, /* copy message */ 1 TSRMLS_CC );
			efree( buf );
		}
		zval_dtor( object );
		return FAILURE;
	}
	
	transliterator_object_construct( object, utrans, TRANSLITERATOR_ERROR_CODE_P( to ) TSRMLS_CC );
	/* no need to close the transliterator manually on construction error */
	if( U_FAILURE( TRANSLITERATOR_ERROR_CODE( to ) ) )
	{
		intl_error_set_code( NULL, TRANSLITERATOR_ERROR_CODE( to ) TSRMLS_CC );
		intl_error_set_custom_msg( NULL,
			"transliterator_create: internal constructor call failed", 0 TSRMLS_CC );
		zval_dtor( object );
		return FAILURE;
	}

	return SUCCESS;
}

/* {{{ proto Transliterator transliterator_create( string id [, int direction ] )
 *     proto Transliterator Transliterator::create( string id [, int direction ] )
 * Opens a transliterator by id.
 */
PHP_FUNCTION( transliterator_create )
{
	char  *str_id;
	int   str_id_len;
	long  direction   = TRANSLITERATOR_FORWARD;
	int res;

	TRANSLITERATOR_METHOD_INIT_VARS;
	
	(void) to; /* unused */

	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s|l",
		&str_id, &str_id_len, &direction ) == FAILURE )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_create: bad arguments", 0 TSRMLS_CC );
		RETURN_NULL();
	}

	object = return_value;
	res = create_transliterator( str_id, str_id_len, direction, object TSRMLS_CC );
	if( res == FAILURE )
		RETURN_NULL();
	
	/* success, leave return_value as it is (set by create_transliterator) */
}
/* }}} */

/* {{{ proto Transliterator transliterator_create_from_rules( string rules [, int direction ] )
 *     proto Transliterator Transliterator::createFromRules( string rules [, int direction ] )
 * Opens a transliterator by id.
 */
PHP_FUNCTION( transliterator_create_from_rules )
{
	char		    *str_rules;
	int             str_rules_len;
	UChar		    *ustr_rules    = NULL;
	int32_t         ustr_rules_len = 0;
	long            direction      = TRANSLITERATOR_FORWARD;
	UParseError     parse_error    = {0, -1};
	UTransliterator *utrans;
	UChar           id[] = {0x52, 0x75, 0x6C, 0x65, 0x73, 0x54, 0x72,
					       0x61, 0x6E, 0x73, 0x50, 0x48, 0x50, 0}; /* RulesTransPHP */
	TRANSLITERATOR_METHOD_INIT_VARS;

	if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s|l",
		&str_rules, &str_rules_len, &direction ) == FAILURE )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_create_from_rules: bad arguments", 0 TSRMLS_CC );
		RETURN_NULL();
	}

	if( ( direction != TRANSLITERATOR_FORWARD ) && (direction != TRANSLITERATOR_REVERSE ) )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_create_from_rules: invalid direction", 0 TSRMLS_CC );
		RETURN_NULL();
	}

	object = return_value;
	object_init_ex( object, Transliterator_ce_ptr );
	TRANSLITERATOR_METHOD_FETCH_OBJECT_NO_CHECK;

	intl_convert_utf8_to_utf16( &ustr_rules, &ustr_rules_len,
		str_rules, str_rules_len, TRANSLITERATOR_ERROR_CODE_P( to ) );
	/* (I'm not a big fan of non-obvious flow control macros ).
	 * This one checks the error value, destroys object and returns false */
	INTL_CTOR_CHECK_STATUS( to, "String conversion of rules to UTF-16 failed" );

	/* Open ICU Transliterator. */
	utrans = utrans_openU( id, ( sizeof( id ) - 1 ) / ( sizeof( *id ) ), (UTransDirection ) direction,
		ustr_rules, ustr_rules_len, &parse_error, TRANSLITERATOR_ERROR_CODE_P( to ) );
	if (ustr_rules) {
		efree( ustr_rules );
	}
	
	intl_error_set_code( NULL, INTL_DATA_ERROR_CODE( to ) TSRMLS_CC );
	if( U_FAILURE( INTL_DATA_ERROR_CODE( to ) ) )
	{
		char *msg = NULL;
		smart_str parse_error_str;
		parse_error_str = intl_parse_error_to_string( &parse_error );
		spprintf( &msg, 0, "transliterator_create_from_rules: unable to "
			"create ICU transliterator from rules (%s)", parse_error_str.c );
		smart_str_free( &parse_error_str );
		if( msg != NULL )
		{
			intl_errors_set_custom_msg( INTL_DATA_ERROR_P( to ), msg, 1 TSRMLS_CC );
			efree( msg );
		}
		zval_dtor( return_value );
		RETURN_NULL();
    }
	transliterator_object_construct( object, utrans, TRANSLITERATOR_ERROR_CODE_P( to ) TSRMLS_CC );
	/* no need to close the transliterator manually on construction error */
	INTL_CTOR_CHECK_STATUS( to, "transliterator_create_from_rules: internal constructor call failed" );
}
/* }}} */

/* {{{ proto Transliterator transliterator_create_inverse( Transliterator orig_trans )
 *     proto Transliterator Transliterator::createInverse()
 * Opens the inverse transliterator transliterator.
 */
PHP_FUNCTION( transliterator_create_inverse )
{
	Transliterator_object *to_orig;
	UTransliterator       *utrans;
	TRANSLITERATOR_METHOD_INIT_VARS;

	if( zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
		&object, Transliterator_ce_ptr ) == FAILURE )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_create_inverse: bad arguments", 0 TSRMLS_CC );
		RETURN_NULL();
	}

	TRANSLITERATOR_METHOD_FETCH_OBJECT;
	to_orig = to;

	object = return_value;
	object_init_ex( object, Transliterator_ce_ptr );
	TRANSLITERATOR_METHOD_FETCH_OBJECT_NO_CHECK; /* change "to" into new object (from "object" ) */
	
	utrans = utrans_openInverse( to_orig->utrans, TRANSLITERATOR_ERROR_CODE_P( to ) );
	INTL_CTOR_CHECK_STATUS( to, "transliterator_create_inverse: could not create "
		"inverse ICU transliterator" );
	transliterator_object_construct( object, utrans, TRANSLITERATOR_ERROR_CODE_P( to ) TSRMLS_CC );
	/* no need to close the transliterator manually on construction error */
	INTL_CTOR_CHECK_STATUS( to, "transliterator_create: internal constructor call failed" );
}
/* }}} */

/* {{{ proto array transliterator_list_ids()
 *     proto array Transliterator::listIDs()
 * Return an array with the registered transliterator IDs.
 */
PHP_FUNCTION( transliterator_list_ids )
{
	UEnumeration  *en;
	const UChar	  *elem;
	int32_t		  elem_len;
	UErrorCode	  status = U_ZERO_ERROR;
	
	intl_error_reset( NULL TSRMLS_CC );

	if( zend_parse_parameters_none() == FAILURE )
	{
		/* seems to be the convention in this lib to return false instead of
		 * null on bad parameter types, except on constructors and factory
		 * methods */
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_list_ids: bad arguments", 0 TSRMLS_CC );
		RETURN_FALSE;
	}

	en = utrans_openIDs( &status );
	INTL_CHECK_STATUS( status,
		"transliterator_list_ids: Failed to obtain registered transliterators" );

	array_init( return_value );
	while( (elem = uenum_unext( en, &elem_len, &status )) )
	{
		char *el_char = NULL;
		int  el_len   = 0;

		intl_convert_utf16_to_utf8( &el_char, &el_len, elem, elem_len, &status );

		if( U_FAILURE( status ) )
		{
			efree( el_char );
			break;
		}
		else
		{
			add_next_index_stringl( return_value, el_char, el_len, 0 );
		}
	}
	uenum_close( en );

	intl_error_set_code( NULL, status TSRMLS_CC );
	if( U_FAILURE( status ) )
	{
		zval_dtor( return_value );
		RETVAL_FALSE;
		intl_error_set_custom_msg( NULL, "transliterator_list_ids: "
			"Failed to build array of registered transliterators", 0 TSRMLS_CC );
	}
}
/* }}} */

/* {{{ proto string transliterator_transliterate( Transliterator trans, string subject [, int start = 0 [, int end = -1 ]] )
 *     proto string Transliterator::transliterate( string subject [, int start = 0 [, int end = -1 ]] )
 * Transliterate a string. */
PHP_FUNCTION( transliterator_transliterate )
{
	char	    *str;
	UChar		*ustr		= NULL,
				*uresult	= NULL;
	int			str_len;
	int32_t		ustr_len	= 0,
				capacity,
				uresult_len;
	long		start		= 0,
				limit		= -1;
	int			success     = 0,
				temp_trans  = 0;
	TRANSLITERATOR_METHOD_INIT_VARS;

	object = getThis();

	if( object == NULL )
	{
		/* in non-OOP version, accept both a transliterator and a string */
		zval **arg1;
		if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "Zs|ll",
			&arg1, &str, &str_len, &start, &limit ) == FAILURE )
		{
			intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
				"transliterator_transliterate: bad arguments", 0 TSRMLS_CC );
			RETURN_FALSE;
		}
		
		if( Z_TYPE_PP( arg1 ) == IS_OBJECT &&
			instanceof_function( Z_OBJCE_PP( arg1 ), Transliterator_ce_ptr TSRMLS_CC ) )
		{
			object = *arg1;
		}
		else
		{ /* not a transliterator object as first argument */
			int res;
			if(Z_TYPE_PP( arg1 ) != IS_STRING )
			{
				SEPARATE_ZVAL( arg1 );
				convert_to_string( *arg1 );
			}
			ALLOC_INIT_ZVAL( object );
			temp_trans = 1;
			res = create_transliterator( Z_STRVAL_PP( arg1 ), Z_STRLEN_PP( arg1 ),
					TRANSLITERATOR_FORWARD, object TSRMLS_CC );
			if( res == FAILURE )
			{
				char *message = intl_error_get_message( NULL TSRMLS_CC );
				php_error_docref0( NULL TSRMLS_CC, E_WARNING, "Could not create "
					"transliterator with ID \"%s\" (%s)", Z_STRVAL_PP( arg1 ), message );
				efree( message );
				/* don't set U_ILLEGAL_ARGUMENT_ERROR to allow fetching of inner error */
				goto cleanup;
			}
		}
	}
	else if( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s|ll",
		&str, &str_len, &start, &limit ) == FAILURE )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_transliterate: bad arguments", 0 TSRMLS_CC );
		RETURN_FALSE;
	}
	
	if( limit < -1 )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_transliterate: \"end\" argument should be "
			"either non-negative or -1", 0 TSRMLS_CC );
		RETURN_FALSE;
	}

	if( start < 0 || ((limit != -1 ) && (start > limit )) )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_transliterate: \"start\" argument should be "
			"non-negative and not bigger than \"end\" (if defined)", 0 TSRMLS_CC );
		RETURN_FALSE;
	}

	/* end argument parsing/validation */

	TRANSLITERATOR_METHOD_FETCH_OBJECT;
	
	intl_convert_utf8_to_utf16( &ustr, &ustr_len, str, str_len,
		TRANSLITERATOR_ERROR_CODE_P( to ) );
	INTL_METHOD_CHECK_STATUS( to, "String conversion of string to UTF-16 failed" );

	/* we've started allocating resources, goto from now on */

	if( ( start > ustr_len ) || (( limit != -1 ) && (limit > ustr_len ) ) )
	{
		char *msg;
		spprintf( &msg, 0,
			"transliterator_transliterate: Neither \"start\" nor the \"end\" "
			"arguments can exceed the number of UTF-16 code units "
			"(in this case, %d)", (int) ustr_len );
		if(msg != NULL )
		{
			intl_errors_set( TRANSLITERATOR_ERROR_P( to ), U_ILLEGAL_ARGUMENT_ERROR,
				msg, 1 TSRMLS_CC );
			efree( msg );
		}
		RETVAL_FALSE;
		goto cleanup;
	}

	uresult = safe_emalloc( ustr_len, sizeof( UChar ), 1 * sizeof( UChar ) );
	capacity = ustr_len + 1;

	while( 1 )
	{
		int32_t temp_limit = ( limit == -1 ? ustr_len : (int32_t) limit );
		memcpy( uresult, ustr, ustr_len * sizeof( UChar ) );
		uresult_len = ustr_len;

		utrans_transUChars( to->utrans, uresult, &uresult_len, capacity, (int32_t) start,
			&temp_limit, TRANSLITERATOR_ERROR_CODE_P( to ) );
		if( TRANSLITERATOR_ERROR_CODE( to ) == U_BUFFER_OVERFLOW_ERROR )
		{
			efree( uresult );

			uresult = safe_emalloc( uresult_len, sizeof( UChar ), 1 * sizeof( UChar ) );
			capacity = uresult_len + 1;

			intl_error_reset( TRANSLITERATOR_ERROR_P( to ) TSRMLS_CC );
		}
		else if(TRANSLITERATOR_ERROR_CODE( to ) == U_STRING_NOT_TERMINATED_WARNING )
		{
			uresult = safe_erealloc( uresult, uresult_len, sizeof( UChar ), 1 * sizeof( UChar ) );
			
			intl_error_reset( TRANSLITERATOR_ERROR_P( to ) TSRMLS_CC );
			break;
		}
		else if( U_FAILURE( TRANSLITERATOR_ERROR_CODE( to ) ) )
		{
			intl_error_set_code( NULL, TRANSLITERATOR_ERROR_CODE( to ) TSRMLS_CC );
			intl_errors_set_custom_msg( TRANSLITERATOR_ERROR_P( to ),
				"transliterator_transliterate: transliteration failed", 0 TSRMLS_CC );
			goto cleanup;
		}
		else
			break;
	}

	uresult[uresult_len] = (UChar) 0;
	
	success = 1;

cleanup:
	if( ustr )
		efree( ustr );

	if( success ) {
		/* frees uresult even on error */
		INTL_METHOD_RETVAL_UTF8( to, uresult, uresult_len, 1 );
	}
	else
	{
		if( uresult )
			efree( uresult );
		RETVAL_FALSE;
	}

	if (temp_trans )
		zval_ptr_dtor( &object );
}
/* }}} */

PHP_METHOD( Transliterator, __construct )
{
	/* this constructor shouldn't be called as it's private */
	zend_throw_exception( NULL,
		"An object of this type cannot be created with the new operator.",
		0 TSRMLS_CC );
}

/* {{{ proto int transliterator_get_error_code( Transliterator trans )
 *     proto int Transliterator::getErrorCode()
 * Get the last error code for this transliterator.
 */
PHP_FUNCTION( transliterator_get_error_code )
{
	TRANSLITERATOR_METHOD_INIT_VARS

	if( zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
		&object, Transliterator_ce_ptr ) == FAILURE )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_get_error_code: unable to parse input params", 0 TSRMLS_CC );

		RETURN_FALSE;
	}

	/* Fetch the object (without resetting its last error code ). */
	to = zend_object_store_get_object( object TSRMLS_CC );
	if (to == NULL )
		RETURN_FALSE;

	RETURN_LONG( (long) TRANSLITERATOR_ERROR_CODE( to ) );
}
/* }}} */


/* {{{ proto string transliterator_get_error_message( Transliterator trans )
 *     proto string Transliterator::getErrorMessage()
 * Get the last error message for this transliterator.
 */
PHP_FUNCTION( transliterator_get_error_message )
{
	const char* message = NULL;
	TRANSLITERATOR_METHOD_INIT_VARS

	if( zend_parse_method_parameters( ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
		&object, Transliterator_ce_ptr ) == FAILURE )
	{
		intl_error_set( NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"transliterator_get_error_message: unable to parse input params", 0 TSRMLS_CC );

		RETURN_FALSE;
	}


	/* Fetch the object (without resetting its last error code ). */
	to = zend_object_store_get_object( object TSRMLS_CC );
	if (to == NULL )
		RETURN_FALSE;

	/* Return last error message. */
	message = intl_error_get_message( TRANSLITERATOR_ERROR_P( to ) TSRMLS_CC );
	RETURN_STRING( message, 0 );
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

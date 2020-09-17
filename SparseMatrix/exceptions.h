/**
 * This file is part of the SparseMatrix library
 *
 * @license  MIT
 * @author   Petr Kessler (https://kesspess.cz)
 * @link     https://github.com/uestla/Sparse-Matrix
 */


#ifndef __SPARSEMATRIX_EXCEPTIONS_H__

	#define	__SPARSEMATRIX_EXCEPTIONS_H__

	#include <exception>


	namespace sparsematrix
	{

		class Exception : public std::exception
		{

			public:

				explicit Exception(const std::string & message) : exception(), message(message)
				{}


				virtual ~Exception(void) throw ()
				{}


				inline std::string getMessage(void) const
				{
					return this->message;
				}


			protected:

				std::string message;

		};


		class InvalidDimensionsException : public Exception
		{

			public:

				InvalidDimensionsException(const std::string & message) : Exception(message)
				{}

		};


		class InvalidCoordinatesException : public Exception
		{

			public:

				InvalidCoordinatesException(const std::string & message) : Exception(message)
				{}

		};

	}

#endif

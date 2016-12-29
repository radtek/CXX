#include "GetOptClass.h"
#include <stdlib.h>
#include <stdio.h>

namespace ZQ {
namespace common {

GetOpt::GetOpt(int arg_c, char **arg_v, const char *opt_string, int _long_only)
{
	argc = arg_c;
	argv = arg_v;
	optstring = opt_string;
	long_options = 0;
	opt_index = 0;
	long_only = _long_only;
	optind = 0;
	__getopt_initialized = 0;
}


GetOpt::GetOpt(int arg_c, char **arg_v, const char *options, const struct option *_long_options, int *_opt_index, int _long_only)
{
	argc = arg_c;
	argv = arg_v;
	optstring = options;
	long_options = _long_options;
	opt_index = _opt_index;
	long_only = _long_only;
	optind = 0;
	__getopt_initialized = 0;
}

GetOpt::GetOpt(char *_argv, const char *opt_string, int _long_only)
{
	std::vector<std::string> arg_v;
	ParseCmdLine(_argv, arg_v);
	argv = new char*[arg_v.size()];
	for(int i = 0; i < arg_v.size(); i++)
	{
		argv[i] = new char[arg_v[i].length()+1];
		strcpy(argv[i], arg_v[i].c_str());
	}
	argc = arg_v.size();
	optstring = opt_string;
	long_options = 0;
	opt_index = 0;
	long_only = _long_only;
	optind = 0;
	__getopt_initialized = 0;
}

GetOpt::GetOpt(std::vector<std::string>& arg_v, const char *opt_string, int _long_only)
{
	argv = new char*[arg_v.size()];
	for(int i = 0; i < arg_v.size(); i++)
	{
		argv[i] = new char[arg_v[i].length()+1];
		strcpy(argv[i], arg_v[i].c_str());
	}
	argc = arg_v.size();
	optstring = opt_string;
	long_options = 0;
	opt_index = 0;
	long_only = _long_only;
	optind = 0;
	__getopt_initialized = 0;
}

GetOpt::~GetOpt(void)
{
}

int GetOpt::_getopt_internal()
{
	int print_errors = opterr;
	if (optstring[0] == ':')
		print_errors = 0;

	if (argc < 1)
		return -1;

	_optarg = NULL;

	if (optind == 0 || !__getopt_initialized)
	{
		if (optind == 0)
			optind = 1;	/* Don't scan ARGV[0], the program name.  */
		optstring = _getopt_initialize (argc, argv, optstring);
		__getopt_initialized = 1;
	}

#define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0')

	if (nextchar == NULL || *nextchar == '\0')
	{
		/* Advance to the next ARGV-element.  */

		/* Give FIRST_NONOPT & LAST_NONOPT rational values if OPTIND has been
		moved back by the user (who may also have changed the arguments).  */
		if (last_nonopt > optind)
			last_nonopt = optind;
		if (first_nonopt > optind)
			first_nonopt = optind;

		if (ordering == PERMUTE)
		{
			/* If we have just processed some options following some non-options,
			exchange them so that the options come first.  */

			if (first_nonopt != last_nonopt && last_nonopt != optind)
				exchange ((char **) argv);
			else if (last_nonopt != optind)
				first_nonopt = optind;

			/* Skip any additional non-options
			and extend the range of non-options previously skipped.  */

			while (optind < argc && NONOPTION_P)
				optind++;
			last_nonopt = optind;
		}

		/* The special ARGV-element `--' means premature end of options.
		Skip it like a null option,
		then exchange with previous non-options as if it were an option,
		then skip everything else like a non-option.  */

		if (optind != argc && !strcmp (argv[optind], "--"))
		{
			optind++;

			if (first_nonopt != last_nonopt && last_nonopt != optind)
				exchange ((char **) argv);
			else if (first_nonopt == last_nonopt)
				first_nonopt = optind;
			last_nonopt = argc;

			optind = argc;
		}

		/* If we have done all the ARGV-elements, stop the scan
		and back over any non-options that we skipped and permuted.  */

		if (optind == argc)
		{
			/* Set the next-arg-index to point at the non-options
			that we previously skipped, so the caller will digest them.  */
			if (first_nonopt != last_nonopt)
				optind = first_nonopt;
			return -1;
		}

		/* If we have come to a non-option and did not permute it,
		either stop the scan or describe it to the caller and pass it by.  */

		if (NONOPTION_P)
		{
			if (ordering == REQUIRE_ORDER)
				return -1;
			_optarg = argv[optind++];
			return 1;
		}

		/* We have found another option-ARGV-element.
		Skip the initial punctuation.  */

		nextchar = (argv[optind] + 1
			+ (long_options != NULL && argv[optind][1] == '-'));
	}

	/* Decode the current option-ARGV-element.  */

	/* Check whether the ARGV-element is a long option.

	If long_only and the ARGV-element has the form "-f", where f is
	a valid short option, don't consider it an abbreviated form of
	a long option that starts with f.  Otherwise there would be no
	way to give the -f short option.

	On the other hand, if there's a long option "fubar" and
	the ARGV-element is "-fu", do consider that an abbreviation of
	the long option, just like "--fu", and not "-f" with arg "u".

	This distinction seems to be the most useful approach.  */

	if (long_options != NULL
		&& (argv[optind][1] == '-'
		|| (long_only && (argv[optind][2] || !strchr(optstring, argv[optind][1])))))
	{
		char *nameend;
		const struct option *p;
		const struct option *pfound = NULL;
		int exact = 0;
		int ambig = 0;
		int indfound = -1;
		int option_index;

		for (nameend = nextchar; *nameend && *nameend != '='; nameend++)
			/* Do nothing.  */
			;

		/* Test all long options for either exact match
		or abbreviated matches.  */
		for (p = long_options, option_index = 0; p->name; p++, option_index++)
			if (!strncmp (p->name, nextchar, nameend - nextchar))
			{
				if ((unsigned int) (nameend - nextchar)
					== (unsigned int) strlen (p->name))
				{
					/* Exact match found.  */
					pfound = p;
					indfound = option_index;
					exact = 1;
					break;
				}
				else if (pfound == NULL)
				{
					/* First nonexact match found.  */
					pfound = p;
					indfound = option_index;
				}
				else if (long_only
					|| pfound->has_arg != p->has_arg
					|| pfound->flag != p->flag
					|| pfound->val != p->val)
					/* Second or later nonexact match found.  */
					ambig = 1;
			}

			if (ambig && !exact)
			{
				if (print_errors)
				{
					fprintf (stderr, "%s: option `%s' is ambiguous\n",argv[0], argv[optind]);
				}
				nextchar += strlen (nextchar);
				optind++;
				optopt = 0;
				return '?';
			}

			if (pfound != NULL)
			{
				option_index = indfound;
				optind++;
				if (*nameend)
				{
					/* Don't test has_arg with >, because some C compilers don't
					allow it to be used on enums.  */
					if (pfound->has_arg)
						_optarg = nameend + 1;
					else
					{
						if (print_errors)
						{
							if (argv[optind - 1][1] == '-')
							{
								fprintf (stderr, "%s: option `--%s' doesn't allow an argument\n",argv[0], pfound->name);
							}
							else
							{
								fprintf (stderr, "%s: option `%c%s' doesn't allow an argument\n",argv[0], argv[optind - 1][0], pfound->name);

							}
						}

						nextchar += strlen (nextchar);

						optopt = pfound->val;
						return '?';
					}
				}
				else if (pfound->has_arg == 1)
				{
					if (optind < argc)
						_optarg = argv[optind++];
					else
					{
						if (print_errors)
						{
							fprintf (stderr,"%s: option `%s' requires an argument\n",argv[0], argv[optind - 1]);
						}
						nextchar += strlen (nextchar);
						optopt = pfound->val;
						return optstring[0] == ':' ? ':' : '?';
					}
				}
				nextchar += strlen (nextchar);
				if (opt_index != NULL)
					*opt_index = option_index;
				if (pfound->flag)
				{
					*(pfound->flag) = pfound->val;
					return 0;
				}
				return pfound->val;
			}

			/* Can't find it as a long option.  If this is not getopt_long_only,
			or the option starts with '--' or is not a valid short
			option, then it's an error.
			Otherwise interpret it as a short option.  */
			if (!long_only || argv[optind][1] == '-'
				|| strchr(optstring, *nextchar) == NULL)
			{
				if (print_errors)
				{
					if (argv[optind][1] == '-')
					{
						/* --option */
						fprintf (stderr, "%s: unrecognized option `--%s'\n",argv[0], nextchar);
					}
					else
					{
						/* +option or -option */
						fprintf (stderr, "%s: unrecognized option `%c%s'\n",argv[0], argv[optind][0], nextchar);
					}

				}
				nextchar = (char *) "";
				optind++;
				optopt = 0;
				return '?';
			}
	}

	/* Look at and handle the next short option-character.  */

	{
		char c = *nextchar++;
		const char *temp = strchr(optstring, c);

		/* Increment `optind' when we start to process its last character.  */
		if (*nextchar == '\0')
			++optind;

		if (temp == NULL || c == ':')
		{
			if (print_errors)
			{
				if (posixly_correct)
				{
					/* 1003.2 specifies the format of this message.  */
					fprintf (stderr, "%s: illegal option -- %c\n", argv[0], c);
				}
				else
				{
					fprintf (stderr, "%s: invalid option -- %c\n", argv[0], c);
				}

			}
			optopt = c;
			return '?';
		}
		/* Convenience. Treat POSIX -W foo same as long option --foo */
		if (temp[0] == 'W' && temp[1] == ';')
		{
			char *nameend;
			const struct option *p;
			const struct option *pfound = NULL;
			int exact = 0;
			int ambig = 0;
			int indfound = 0;
			int option_index;

			/* This is an option that requires an argument.  */
			if (*nextchar != '\0')
			{
				_optarg = nextchar;
				/* If we end this ARGV-element by taking the rest as an arg,
				we must advance to the next element now.  */
				optind++;
			}
			else if (optind == argc)
			{
				if (print_errors)
				{
					/* 1003.2 specifies the format of this message.  */
					fprintf (stderr, "%s: option requires an argument -- %c\n",argv[0], c);
				}
				optopt = c;
				if (optstring[0] == ':')
					c = ':';
				else
					c = '?';
				return c;
			}
			else
				/* We already incremented `optind' once;
				increment it again when taking next ARGV-elt as argument.  */
				_optarg = argv[optind++];

			/* _optarg is now the argument, see if it's in the
			table of long_options.  */

			for (nextchar = nameend = _optarg; *nameend && *nameend != '='; nameend++)
				/* Do nothing.  */
				;

			/* Test all long options for either exact match
			or abbreviated matches.  */
			for (p = long_options, option_index = 0; p->name; p++, option_index++)
				if (!strncmp (p->name, nextchar, nameend - nextchar))
				{
					if ((unsigned int) (nameend - nextchar) == strlen (p->name))
					{
						/* Exact match found.  */
						pfound = p;
						indfound = option_index;
						exact = 1;
						break;
					}
					else if (pfound == NULL)
					{
						/* First nonexact match found.  */
						pfound = p;
						indfound = option_index;
					}
					else
						/* Second or later nonexact match found.  */
						ambig = 1;
				}
				if (ambig && !exact)
				{
					if (print_errors)
					{
						fprintf (stderr, "%s: option `-W %s' is ambiguous\n",argv[0], argv[optind]);
					}
					nextchar += strlen (nextchar);
					optind++;
					return '?';
				}
				if (pfound != NULL)
				{
					option_index = indfound;
					if (*nameend)
					{
						/* Don't test has_arg with >, because some C compilers don't
						allow it to be used on enums.  */
						if (pfound->has_arg)
							_optarg = nameend + 1;
						else
						{
							if (print_errors)
							{
								fprintf (stderr, "%s: option `-W %s' doesn't allow an argument\n",argv[0], pfound->name);

							}

							nextchar += strlen (nextchar);
							return '?';
						}
					}
					else if (pfound->has_arg == 1)
					{
						if (optind < argc)
							_optarg = argv[optind++];
						else
						{
							if (print_errors)
							{
								fprintf (stderr,"%s: option `%s' requires an argument\n",argv[0], argv[optind - 1]);
							}
							nextchar += strlen (nextchar);
							return optstring[0] == ':' ? ':' : '?';
						}
					}
					nextchar += strlen (nextchar);
					if (opt_index != NULL)
						*opt_index = option_index;
					if (pfound->flag)
					{
						*(pfound->flag) = pfound->val;
						return 0;
					}
					return pfound->val;
				}
				nextchar = NULL;
				return 'W';	/* Let the application handle it.   */
		}
		if (temp[1] == ':')
		{
			if (temp[2] == ':')
			{
				/* This is an option that accepts an argument optionally.  */
				if (*nextchar != '\0')
				{
					_optarg = nextchar;
					optind++;
				}
				else
					_optarg = NULL;
				nextchar = NULL;
			}
			else
			{
				/* This is an option that requires an argument.  */
				if (*nextchar != '\0')
				{
					_optarg = nextchar;
					/* If we end this ARGV-element by taking the rest as an arg,
					we must advance to the next element now.  */
					optind++;
				}
				else if (optind == argc)
				{
					if (print_errors)
					{
						/* 1003.2 specifies the format of this message.  */
						fprintf (stderr,"%s: option requires an argument -- %c\n",argv[0], c);
					}
					optopt = c;
					if (optstring[0] == ':')
						c = ':';
					else
						c = '?';
				}
				else
					/* We already incremented `optind' once;
					increment it again when taking next ARGV-elt as argument.  */
					_optarg = argv[optind++];
				nextchar = NULL;
			}
		}
		return c;
	}
}


const char * GetOpt::_getopt_initialize (int argc, char *const *argv, const char *optstring)
{
	/* Start processing options with ARGV-element 1 (since ARGV-element 0
	is the program name); the sequence of previously skipped
	non-option ARGV-elements is empty.  */

	first_nonopt = last_nonopt = optind;

	nextchar = NULL;

	posixly_correct = getenv ("POSIXLY_CORRECT");

	/* Determine how to handle the ordering of options and nonoptions.  */

	if (optstring[0] == '-')
	{
		ordering = RETURN_IN_ORDER;
		++optstring;
	}
	else if (optstring[0] == '+')
	{
		ordering = REQUIRE_ORDER;
		++optstring;
	}
	else if (posixly_correct != NULL)
		ordering = REQUIRE_ORDER;
	else
		ordering = PERMUTE;
	return optstring;
}

void GetOpt::exchange (char **argv)
{
	int bottom = first_nonopt;
	int middle = last_nonopt;
	int top = optind;
	char *tem;

	/* Exchange the shorter segment with the far end of the longer segment.
	That puts the shorter segment into the right place.
	It leaves the longer segment in the right place overall,
	but it consists of two parts that need to be swapped next.  */

	while (top > middle && middle > bottom)
	{
		if (top - middle > middle - bottom)
		{
			/* Bottom segment is the short one.  */
			int len = middle - bottom;
			register int i;

			/* Swap it with the top part of the top segment.  */
			for (i = 0; i < len; i++)
			{
				tem = argv[bottom + i];
				argv[bottom + i] = argv[top - (middle - bottom) + i];
				argv[top - (middle - bottom) + i] = tem;
			}
			/* Exclude the moved bottom segment from further swapping.  */
			top -= len;
		}
		else
		{
			/* Top segment is the short one.  */
			int len = top - middle;
			register int i;

			/* Swap it with the bottom part of the bottom segment.  */
			for (i = 0; i < len; i++)
			{
				tem = argv[bottom + i];
				argv[bottom + i] = argv[middle + i];
				argv[middle + i] = tem;
			}
			/* Exclude the moved top segment from further swapping.  */
			bottom += len;
		}
	}

	/* Update records for the slots the non-options now occupy.  */

	first_nonopt += (optind - last_nonopt);
	last_nonopt = optind;
}

int GetOpt::hasNext()
{
	_opt = _getopt_internal();
	if(_opt >= 0)
		return _opt;
	else
		return 0;
}

bool GetOpt::ParseCmdLine(const std::string& str ,std::vector<std::string>& result)
{
	result.clear ();
	if(str.empty ())
	{		
		return false;
	}

	const std::string delimiter=" \t";
	const std::string trimDelimiter=" \t";
	const std::string quote="\"";
	const std::string escape="\\";

	std::string replica = str;

	//ȥ����߶���Ŀո����TAB
	if(POSOK(trimDelimiter.find(replica[0])))
	{
		int index;
		for(index=0; index<replica.size(); index++)
		{
			if(!POSOK(trimDelimiter.find(replica[index])))
			{
				replica = replica.substr(index, replica.size());
				break;
			}
		}
		if(index >= str.size())  //ȫ���ǿո����TAB����
			return false;
	}

	int		curPos=0;	//current position
	int		lastPos=0;	//last position

	int		length=(int)replica.length ();
	char	ch;
	const char*	pStrPointer=replica.c_str();	
	bool bInQuotes = false;
	std::string quotes;
	std::string args;

	while ( length > curPos )
	{
		ch=pStrPointer[curPos];
	
		if(!bInQuotes && POSOK(trimDelimiter.find(ch))) // ȥ���������ж���Ŀո����TAB
		{
			if(curPos-1 >= 0 && POSOK(trimDelimiter.find(pStrPointer[curPos-1]))) // ǰһ���ַ��ǿո����TAB
			{
				curPos++;
				continue;
			}
			else if(curPos-1 >= 0 && !POSOK(trimDelimiter.find(pStrPointer[curPos-1]))) // ǰһ���ַ����ǿո����TAB
			{
				if(!args.empty())
				{
					result.push_back(args);
					args.clear();
				}
				curPos++;
				continue;
			}
		}
		if(POSOK(quote.find(ch)))   // ��������÷�"
		{
			if(!bInQuotes) //������
			{
				bInQuotes = true;
				curPos++;
				continue;
			} 
			else //����
			{
				bInQuotes = false;
				curPos++;
				args = args + quotes;
				quotes.clear();
				continue;		
			}
		}
		if(POSOK(escape.find(ch)))  //�����backslash
		{
			if((curPos+1 < length && POSOK(quote.find(pStrPointer[curPos+1])))) //backslash����"
			{
				curPos = curPos + 2;
				if(bInQuotes)
					quotes.push_back('"');
				else
					args.push_back('"');
				continue;
			}	
			else
			{
				int count = -1;
				while(ch=='\\')
				{
					ch = pStrPointer[curPos++];
					count++;
				}
				if(ch == '"')
				{
					if(count%2)	//������\���"
					{
						if(bInQuotes)
						{
							while(count = count / 2)
							{
								quotes.push_back('\\');
							}	
							quotes.push_back('"');
							bInQuotes = false;
							args = args + quotes;
							quotes.clear();
						}
						else
						{
							while(count = count / 2)
							{
								args.push_back('\\');
							}	
							args.push_back('"');
						}			
						continue;
					}
					else		//ż����\���"
					{
						while(count = count / 2)
						{
							if(bInQuotes)
								quotes.push_back('\\');
							else
								args.push_back('\\');
						}
						if(bInQuotes)
						{
							bInQuotes = false;
							args = args + quotes;
							quotes.clear();
						}
						else
						{
							bInQuotes = true;
						}
						continue;
					}
				}
				else
				{
					while(count--)
					{
						if(bInQuotes)
							quotes.push_back('\\');
						else
							args.push_back('\\');
					}
					
					if(bInQuotes)
						quotes.push_back(ch);
					else
					{
						if(POSOK(delimiter.find(ch)))
						{
							if(!args.empty())
							{
								result.push_back(args);
								args.clear();
							}
							continue;
						}
						else
							args.push_back(ch);
					}
					continue;
				}
			}
		}

		if(bInQuotes)
			quotes.push_back(ch);
		else
			args.push_back(ch);

		curPos++;
		
		if(curPos == length)
		{
			if(!quotes.empty())
				args = args + quotes;
			if(!args.empty())
				result.push_back(args);
		}
	}
	return true;
} // ParseCmdLine()
}//common
}//ZQ 
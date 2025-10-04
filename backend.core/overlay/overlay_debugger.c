
#include "interpreter_config.h"
#include "overlay.h"
#include "overlay_debugger.h"
#include "core.h"
#include "rcstring.h"
#include "text_lib.h"
#include "tokenizer.h"
#include "token.h"
#include "charsets.h"
#include "variables.h"
#include "string_utils.h"
#include "rcstring.h"
#include "machine.h"
#include "globmatch.h"
#include <string.h>

void new_line(struct Core *core)
{
	struct TextLib *lib = &core->overlay->textLib;

	lib->cursorX = 0;
	lib->cursorY++;
	txtlib_scrollWindowIfNeeded(lib);
}

void print_value(struct Core *core, enum ValueType type, union Value *value)
{
	if (type == ValueTypeFloat && value)
	{
		char buffer[20];
		snprintf(buffer, 20, "  %0.10g", value->floatValue);
		txtlib_printText(&core->overlay->textLib, buffer);
		new_line(core);
	}
	else if (type == ValueTypeString && value)
	{
		txtlib_printText(&core->overlay->textLib, "  \"");
		txtlib_printText(&core->overlay->textLib, value->stringValue->chars);
		txtlib_printText(&core->overlay->textLib, "\"");
		new_line(core);
	}
	else
	{
		txtlib_printText(&core->overlay->textLib, "  variable not found");
		new_line(core);
	}
}

void set_value(struct Core *core, enum ValueType type, union Value *value, enum TokenType newType, float newFloat, struct RCString *newString)
{
	// NOTE: value and newValue are alreay tested before arriving here
	if (newType == TokenFloat && type == ValueTypeFloat)
	{
		value->floatValue = newFloat;
	}
	else if (newType == TokenString && type == ValueTypeString)
	{
		if (value->stringValue)
			rcstring_release(value->stringValue);
		value->stringValue = newString;
	}
	else
	{
		txtlib_printText(&core->overlay->textLib, "  type mismatch");
		new_line(core);
		return;
	}
}

void print_command_line(struct Core *core)
{
	struct TextLib *lib = &core->overlay->textLib;
	struct Overlay *overlay = core->overlay;

	txtlib_setCells(lib, 0, lib->windowY + lib->cursorY, 27, lib->windowY + lib->cursorY, 0);
	txtlib_writeText(lib, overlay->commandLine, lib->windowX, lib->windowY + lib->cursorY);
}

struct SimpleVariable *get_simple_var(struct Core *core, const char *looking_name)
{
	struct Tokenizer *tokenizer = &core->interpreter->tokenizer;
	for (int i = 0; i < MAX_SYMBOLS && tokenizer->symbols[i].name[0] != 0; i++)
		if (strcmp(looking_name, tokenizer->symbols[i].name) == 0)
			return var_getSimpleVariable(core->interpreter, i, core->interpreter->subLevel);
	return NULL;
}

struct ArrayVariable *get_array_var(struct Core *core, const char *looking_name)
{
	struct Tokenizer *tokenizer = &core->interpreter->tokenizer;
	for (int i = 0; i < MAX_SYMBOLS && tokenizer->symbols[i].name[0] != 0; i++)
		if (strcmp(looking_name, tokenizer->symbols[i].name) == 0)
			return var_getArrayVariable(core->interpreter, i, core->interpreter->subLevel);
	return NULL;
}

int get_address(struct Core *core, struct Token *t)
{
	if (t->type != TokenFloat)
	{
		txtlib_printText(&core->overlay->textLib, "  syntax error");
		new_line(core);
		return -1;
	}
	int address = (int)t->floatValue;
	if (address < 0 || address >= 0x10000)
	{
		txtlib_printText(&core->overlay->textLib, "  out of bounds");
		new_line(core);
		return -1;
	}
	return address;
}

void process_command_line(struct Core *core)
{
	struct Tokenizer toks;
	struct CoreError err;
	memset(&toks, 0, sizeof(struct Tokenizer));

	err = tok_tokenizeUppercaseProgram(&toks, core->overlay->commandLine);

	size_t i = 0;
	struct Token *t;
	if (err.code == ErrorNone && toks.numTokens > 0)
	{
		t = &toks.tokens[i++];
		if (t->type == TokenEol)
		{
			return;
		}
		else if (t->type == TokenIdentifier || t->type == TokenStringIdentifier)
		{
			t = &toks.tokens[i++];
			struct SimpleVariable *simple = get_simple_var(core, toks.symbols[0].name);
			struct ArrayVariable *array = get_array_var(core, toks.symbols[0].name);
			if (!simple && !array)
				print_value(core, ValueTypeNull, &simple->v);
			// read simple variable
			else if (simple && t->type == TokenEol)
				print_value(core, simple->type, &simple->v);
			// write simple variable
			else if (simple && t->type == TokenEq)
			{
				t = &toks.tokens[i++];
				set_value(core, simple->type, &simple->v, t->type, t->floatValue, t->stringValue);
			}
			else if (array && t->type == TokenEol)
			{
				// TODO: what to do here?
			}
			else if (array && t->type == TokenBracketOpen)
			{
				t = &toks.tokens[i++];
				int indices[MAX_ARRAY_DIMENSIONS], dimensions = 0;
				while (t->type != TokenBracketClose && t->type != TokenEol && t->type != TokenEq && dimensions < MAX_ARRAY_DIMENSIONS)
				{
					if (t->type == TokenFloat)
					{
						if (t->floatValue < 0 || t->floatValue >= array->dimensionSizes[dimensions])
						{
							txtlib_printText(&core->overlay->textLib, "  out of bounds");
							new_line(core);
							return;
						}
						indices[dimensions++] = t->floatValue;
						t = &toks.tokens[i++];
					}
					else
					{
						txtlib_printText(&core->overlay->textLib, "  syntax error");
						new_line(core);
						return;
					}
					if (t->type != TokenComma && t->type != TokenBracketClose)
					{
						txtlib_printText(&core->overlay->textLib, "  syntax error");
						new_line(core);
						return;
					}
					t = &toks.tokens[i++];
				}
				if (dimensions != array->numDimensions)
				{
					txtlib_printText(&core->overlay->textLib, "  wrong dimensions");
					new_line(core);
					return;
				}
				union Value *value = var_getArrayValue(core->interpreter, array, &indices[0]);
				// read array item
				if (t->type == TokenEol)
					print_value(core, array->type, value);
				// write array item
				else if (t->type == TokenEq)
				{
					t = &toks.tokens[i++];
					set_value(core, array->type, value, t->type, t->floatValue, t->stringValue);
				}
			}
		}

		// resume execution
		else if (t->type == TokenPAUSE)
		{
			overlay_clear(core);
			core->overlay->textLib.cursorX = 0;
			core->overlay->textLib.cursorY = 0;
			core->interpreter->state = StateEvaluate;

			struct ControlsInfo info;
			info.keyboardMode = KeyboardModeOff;
			core->delegate->controlsDidChange(core->delegate->context, info);
		}

		// clear screen
		else if (t->type == TokenCLS)
		{
			overlay_clear(core);
			core->overlay->textLib.cursorX = 0;
			core->overlay->textLib.cursorY = 0;
		}

		// resume execution until next WAIT
		else if (t->type == TokenWAIT)
		{
			core->interpreter->pauseAtWait = true;
			core->interpreter->state = StateEvaluate;
		}

		// list variables
		else if (t->type == TokenDIM)
		{
			char filter[SYMBOL_NAME_SIZE+2] = "*";
			int pagination = 0;
			t = &toks.tokens[i++];
			if (t->type == TokenIdentifier || t->type == TokenStringIdentifier)
			{
				strcat(filter, toks.symbols[t->symbolIndex].name);
				strcat(filter, "*");
				t = &toks.tokens[i++];
			}
			if (t->type == TokenFloat && t->floatValue >= 1) pagination = (int)t->floatValue;
			struct Tokenizer *tokenizer = &core->interpreter->tokenizer;
			int canPrint = (core->overlay->textLib.windowHeight - 3);
			int printed = 0;
			// TODO: get shown.h and safe.top and keyboard.height
			for (int i = 0; i < MAX_SYMBOLS && tokenizer->symbols[i].name[0] != 0; i++)
			{
				struct SimpleVariable *simple = var_getSimpleVariable(core->interpreter, i, core->interpreter->subLevel);
				struct ArrayVariable *array = var_getArrayVariable(core->interpreter, i, core->interpreter->subLevel);
				if (!simple && !array)
					continue;
				else if (pagination == 0 && printed >= canPrint)
				{
					txtlib_printText(&core->overlay->textLib, "  ...");
					new_line(core);
					break;
				}
				else if (pagination > 0 && printed < canPrint)
					printed++;
				else if (pagination > 0)
				{
					pagination--;
					printed = 0;
				}
				if (simple && pagination == 0 && sl_globmatch(tokenizer->symbols[i].name, filter)>0)
				{
					txtlib_printText(&core->overlay->textLib, "  ");
					txtlib_printText(&core->overlay->textLib, tokenizer->symbols[i].name);
					new_line(core);
					printed++;
				}
				else if (array && pagination == 0 && sl_globmatch(tokenizer->symbols[i].name, filter)>0)
				{
					txtlib_printText(&core->overlay->textLib, "  ");
					txtlib_printText(&core->overlay->textLib, tokenizer->symbols[i].name);
					for (int j = 0; j < array->numDimensions; j++)
					{
						char buffer[20];
						snprintf(buffer, 20, ",%d", array->dimensionSizes[j] - 1);
						if (j == 0)
							buffer[0] = '(';
						txtlib_printText(&core->overlay->textLib, buffer);
					}
					txtlib_printText(&core->overlay->textLib, ")");
					new_line(core);
					printed++;
				}
			}
		}

		else if (t->type == TokenFloat)
		{
			int address = (int)t->floatValue;
			if (address < 0 || address >= 0x10000)
			{
				txtlib_printText(&core->overlay->textLib, "  out of bounds");
				new_line(core);
				return;
			}
			t = &toks.tokens[i++];
			// read memory
			if (t->type == TokenEol)
			{
				int peek = machine_peek(core, address);
				if (peek == -1)
				{
					txtlib_printText(&core->overlay->textLib, "  illegal memory access");
					new_line(core);
					return;
				}
				struct TypedValue value;
				value.type = ValueTypeFloat;
				value.v.floatValue = peek;
				print_value(core, ValueTypeFloat, &value.v);
			}
			// write memory
			else if (t->type == TokenEq)
			{
				t = &toks.tokens[i++];
				if (t->type != TokenFloat)
				{
					txtlib_printText(&core->overlay->textLib, "  syntax error");
					new_line(core);
					return;
				}
				int poke = machine_poke(core, address, (int)t->floatValue);
				if (!poke)
				{
					txtlib_printText(&core->overlay->textLib, "  illegal memory access");
					new_line(core);
					return;
				}
			}
		}

		// show stack trace
		else if (t->type == TokenTRACE)
		{
			char buffer[20];
			for(int i=0; i<core->interpreter->numLabelStackItems; ++i)
			{
				txtlib_printText(&core->overlay->textLib,"  ");

				char *ptr=(char*)(&core->interpreter->sourceCode[core->interpreter->labelStackItems[i].token->sourcePosition-1]);
				while((*ptr>='a' && *ptr<='z')
				|| (*ptr>='A' && *ptr<='Z')
				|| (*ptr>='0' && *ptr<='9')
				|| *ptr=='_')
				{
					ptr--;
				}
				size_t len=&core->interpreter->sourceCode[core->interpreter->labelStackItems[i].token->sourcePosition-1]-ptr;
				if(len>20) len=20;
				buffer[len]='\0';
				memcpy(&buffer,ptr+1,len);
				txtlib_printText(&core->overlay->textLib,buffer);
				new_line(core);
			}
			new_line(core);
		}

		else
		{
			txtlib_printText(&core->overlay->textLib, "  unsupported keyword");
			new_line(core);
			return;
		}
	}
}

void overlay_debugger(struct Core *core)
{
	struct TextLib *lib = &core->overlay->textLib;
	struct Overlay *overlay = core->overlay;

	struct ControlsInfo info;
	info.keyboardMode = KeyboardModeOn;
	core->delegate->controlsDidChange(core->delegate->context, info);

	char key = core->machine->ioRegisters.key;
	if (key && lib->cursorX < 27)
	{
		core->machine->ioRegisters.key = 0;

		if (key == CoreInputKeyBackspace)
		{
			if (lib->cursorX > 0 && strlen(overlay->commandLine) > 0)
			{
				// move chars after the cursor one position to the left
				memmove(overlay->commandLine + lib->cursorX - 1, overlay->commandLine + lib->cursorX, strlen(overlay->commandLine) - lib->cursorX + 1);
				lib->cursorX--;
				print_command_line(core);
			}
		}
		else if (key == CoreInputKeyDelete)
		{
			if (strlen(overlay->commandLine) - lib->cursorX > 0)
			{
				// move chars after the cursor one position to the left
				memmove(overlay->commandLine + lib->cursorX, overlay->commandLine + lib->cursorX + 1, strlen(overlay->commandLine) - lib->cursorX);
				print_command_line(core);
			}
		}
		else if (key == CoreInputKeyLeft)
		{
			if (lib->cursorX > 0)
			{
				lib->cursorX--;
				print_command_line(core);
			}
		}
		else if (key == CoreInputKeyRight)
		{
			if (lib->cursorX < 26 && lib->cursorX < strlen(overlay->commandLine))
			{
				lib->cursorX++;
				print_command_line(core);
			}
		}
		else if (key == CoreInputKeyReturn)
		{
			if (strlen(overlay->commandLine) > 0)
			{
				print_command_line(core);
				new_line(core);
				strcpy(overlay->previousCommandLine[overlay->previouscommandLineWriteIndex++], overlay->commandLine);
				if (overlay->previouscommandLineWriteIndex >= 9)
					overlay->previouscommandLineWriteIndex = 0;
				overlay->previouscommandLineReadIndex = overlay->previouscommandLineWriteIndex;
				process_command_line(core);
				memset(core->overlay->commandLine, 0, 27);
			}
		}
		else if (key == CoreInputKeyUp)
		{
			if (strlen(overlay->previousCommandLine[(overlay->previouscommandLineReadIndex + 9 - 1) % 9]) > 0)
			{
				overlay->previouscommandLineReadIndex = (overlay->previouscommandLineReadIndex + 9 - 1) % 9;
				strcpy(overlay->commandLine, overlay->previousCommandLine[overlay->previouscommandLineReadIndex]);
				lib->cursorX = (int)strlen(overlay->commandLine);
				print_command_line(core);
			}
		}
		else if (key == CoreInputKeyDown)
		{
			if (strlen(overlay->previousCommandLine[(overlay->previouscommandLineReadIndex + 1) % 9]) > 0)
			{
				overlay->previouscommandLineReadIndex = (overlay->previouscommandLineReadIndex + 1) % 9;
				strcpy(overlay->commandLine, overlay->previousCommandLine[overlay->previouscommandLineReadIndex]);
				lib->cursorX = (int)strlen(overlay->commandLine);
				print_command_line(core);
			}
		}
		else
		{
			// insert char into the command line buffer
			if (lib->cursorX < 26)
			{
				char tmp[27];
				strncpy(tmp, overlay->commandLine, 27);
				memcpy(overlay->commandLine + lib->cursorX + 1, tmp + lib->cursorX, strlen(tmp) - lib->cursorX);
				overlay->commandLine[lib->cursorX++] = key;
				print_command_line(core);
			}
		}
	}
}

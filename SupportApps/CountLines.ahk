; I know; lines of code aren't a good metric, blah blah blah
; I would still like to know how many I've written :)
; This script isn't perfect and I'm sure there's many other/simpler ways to do this, but if that appealed to me, would I be writing an OS? :P

TotalFiles := 0
TotalDirectories := 0
SourceFiles := 0
SourceFilesCounted := 0
NonSourceExts = dll
ExtensionsToParse = cs,c,cpp,h,ahk,hpp,cc
;ExcludeFolders = SDL,My-Doom
SourceOnlyLines := 0
CommentOnlyLines := 0
SourceLinesWithComments := 0
BlankLines := 0
BracesOnlyLines := 0
SkippedLines := 0
TODOs := 0
HACKs := 0
Directories :=
SourceFileList :=
Comments := 

SetWorkingDir, %A_ScriptDir%\..

; Determine how many files and directories we need to scan
Loop, Files, *, DFR
{
	; Don't include files or folders which mostly contain source I didn't write
	IfInString, A_LoopFileFullPath,SDL\
		continue
	IfInString A_LoopFileFullPath,My-Doom\
		continue
	IfInString A_LoopFileFullPath,.git
		continue
	IfInString A_LoopFileFullPath,.vs
		continue
	IfInString A_LoopFileFullPath,font
		continue
	IfInString A_LoopFileFullPath,spf
		continue
	IfInString A_LoopFileFullPath,Debug
		continue
	IfInString A_LoopFileFullPath,Release
		continue
	IfInString A_LoopFileFullPath,x64
		continue
	
	; Don't include generated files
	IfInString, A_LoopFileFullPath,TemporaryGeneratedFile
		continue
		
	FileGetAttrib, Attributes, %A_LoopFileFullPath%
	IfInString, Attributes, D
	{
		TotalDirectories += 1
		Directories := Directories . "`r`n" . A_LoopFileFullPath
	}
	else
	{				
		TotalFiles += 1
		
		if A_LoopFileExt in %ExtensionsToParse%
		{
			SourceFiles += 1
			SourceFileList := SourceFileList . "`r`n" . A_LoopFileFullPath
		}
		else
		{
			if (A_LoopFileExt <> "")
			{
				if A_LoopFileExt not in %NonSourceExts%
					NonSourceExts = %NonSourceExts%,%A_LoopFileExt%
			}
		}
	}
}

; Display a window with a progress bar
;totalEverything := TotalFiles + TotalDirectories
Progress, R0-%SourceFiles%, %a_index%, %a_loopfilename%, Counting..., Counting Lines
Loop, Files, *, DFR
{
	Progress, %SourceFilesCounted%, %a_loopfilename%, Counting..., Counting Lines
	IfNotInString, Attributes, D
	{
		; Don't include files or folders which mostly contain source I didn't write
		IfInString, A_LoopFileFullPath,SDL\
			continue
		IfInString A_LoopFileFullPath,My-Doom\
			continue
		IfInString A_LoopFileFullPath,.git
			continue
		IfInString A_LoopFileFullPath,.vs
			continue
		IfInString A_LoopFileFullPath,font
			continue
		IfInString A_LoopFileFullPath,spf
			continue
		IfInString A_LoopFileFullPath,Debug
			continue
		IfInString A_LoopFileFullPath,Release
			continue
		IfInString A_LoopFileFullPath,x64
			continue
			
		; Don't include generated files
		IfInString, A_LoopFileFullPath,TemporaryGeneratedFile
			continue
		
		if A_LoopFileExt in %ExtensionsToParse%
		{
			CountLines(A_LoopFileFullPath)
			SourceFilesCounted += 1
		}
		else
		{
			if (A_LoopFileExt <> "")
			{
				if A_LoopFileExt not in %NonSourceExts%
					NonSourceExts = %NonSourceExts%,%A_LoopFileExt%
			}
		}
	}
}
Progress, Off

; Format number with comma (From https://autohotkey.com/board/topic/41587-question-about-formating-numbers-1000-to-1000-how/ )
VarSetCapacity( StringLines,32 )
DllCall( "GetNumberFormat",UInt,0x0409,UInt,0,Str,SourceOnlyLines,UInt,0,Str,StringLines,Int,32 )
StringTrimRight, StringLines, StringLines, 3
;MsgBox, % StringLines

;MsgBox %Directories%
CountSummary :=
CountSummary .= NumericalFormat(SourceOnlyLines + SourceLinesWithComments + CommentOnlyLines) . " Total Unique and meaningful lines`r`r"
CountSummary .= NumericalFormat(SourceOnlyLines + SourceLinesWithComments) . " Total lines with code`r"
CountSummary .= NumericalFormat(CommentOnlyLines + SourceLinesWithComments) . " Total lines with comments`r`r"
CountSummary .= NumericalFormat(SourceOnlyLines) . " lines with only code`r"
CountSummary .= NumericalFormat(SourceLinesWithComments) . " lines with comments and code`r"
CountSummary .= NumericalFormat(CommentOnlyLines) . " lines with only comments`r"
CountSummary .= NumericalFormat(BracesOnlyLines) . " lines with only braces`r"
CountSummary .= NumericalFormat(BlankLines) . " blank lines"

CommentOnlyLines := 0
SourceLinesWithComments := 0
BlankLines := 0
BracesOnlyLines := 0

; This can be used for debugging. Here I've set it out to show me lines with comments and code:
;Gui, Add, Edit, R50 w900 Multi vListingBox
;GuiControl,, ListingBox, %Comments%
;GuiControl,, ListingBox, %Directories%
;Gui, Show

MsgBox Results of scanning %A_WorkingDir%:`r`r%CountSummary%`r%TODOs% TODOs`r%HACKs% HACKs`r`rCounted in `r%TotalDirectories% directories`r%SourceFiles% source files
;`rNon-Source files with extensions of %NonSourceExts%
ExitApp ; Replace this with return during development


CountLines(sourceFile)
{
	global
	local line
	local inComment := false
	local maxLines := 0
	
	hasComment := false
	
	Loop, Read, %sourceFile%
	{
		MaxLines += 1
	}
	
	;Progress, R1-%MaxLines%, , %a_loopfilename%, Counting..., Counting Lines	
	Loop, Read, %sourceFile%
	{

	;Progress, %A_Index%, %A_Index%, Counting..., Counting %sourceFile% - %A_Index%
		line := Trim(A_LoopReadLine)
		if (line <> "")
		{
			inComment := DetermineIfInComment(line, inComment)
			
			;MsgBox %line%`r%inComment%
			
			if(inComment)
			{
				;MsgBox In Comment
				if(hasCode)
				{
					SourceLinesWithComments += 1
					Comments .= "`r`n" . line
				}
				else
					CommentOnlyLines += 1
			}
			else
			{
				; not in a comment
				if(hasComment)
				{
					if(hasCode)
					{
						SourceLinesWithComments += 1
						Comments .= "`r`n" . line
					}
					else
					{
						CommentOnlyLines += 1
						;Comments .= "`r`n" . line
					}
						
				}
				else
				{
					; no multi-block comments started here. 
					; check for bracket-only lines
					if(line <> "}" and line <> "{" and line <> "};")
					{
						; line has more than brackets, check for inline comments
						IfInString, line,//
						{
							if(InStr(line, "//") = 1)
								CommentOnlyLines += 1
							else
							{
								SourceLinesWithComments += 1
								Comments .= "`r`n" . line
							}
						}
						else
						{
							SourceOnlyLines += 1
						}
						
						IfInString line,HACK
							HACKs += 1
						IfInString line,TODO
							TODOs += 1
					}
					else
						BracesOnlyLines += 1
				}
			}
		}
		else
			BlankLines += 1
	}
}

NumericalFormat(num)
{
	local returnString
	; Format number with comma (From https://autohotkey.com/board/topic/41587-question-about-formating-numbers-1000-to-1000-how/ )
	VarSetCapacity( returnString,32 )
	DllCall( "GetNumberFormat",UInt,0x0409,UInt,0,Str,num,UInt,0,Str,returnString,Int,32 )
	StringTrimRight, returnString, returnString, 3
	;MsgBox, % StringLines
	return returnString
}

DetermineIfInComment(line, inComment)
{
	beginComment := "/*"
	endComment := "*/"
	global hasCode := false
	global hasComment := false
	while( strlen(line) <> 0 )
	{
		if(inComment)
		{
			; get the next occurrence of end comment
			StringGetPos, EndPos, line,%endComment%
			if(ErrorLevel = 1)
			{
				; couldn't find end of comment, so we're still in a comment
				hasComment := true
				return true
			}
			
			if(EndPos <> 0)
				hasComment := true
			
			inComment := false
			
			endPos += 3
			
			;msgbox %line%`rEnd: %endpos%			
			line := SubStr(line, EndPos)			
			;msgBox %line%
		}
		else
		{
			; check for the next occurrence of begin comment
			StringGetPos, BeginPos, line,%beginComment%
			if(ErrorLevel = 1)
			{
				; couldn't find beginning of comment, so we aren't in a comment
				hasCode := true
				return false
			}
			
			if(BeginPos <> 0)
				hasCode := true
			
			BeginPos += 3
			
			inComment := true
			
			;length := strlen(line) - BeginPos			
			;msgbox %line%`rBegin: %beginpos%`rLength: %Length%			
			line := SubStr(line, BeginPos)			
			;msgBox %line%
		}
	}
	return inComment
}

; Super-useful: reload the script everytime I press ctrl-s (every time I save the script I re-run it)
~^s::
Reload
Exit
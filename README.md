# Alan - implementation of Turing machine

Simple tool implementing turing machine. The program has only one input and that is file fith machine's definition. Purpose of 'Alan' is a job interview assignment.


## Simple description 
We assume the set of symbols is the set of printable ASCII characters, excluding '*' and white-space characters. The blank symbol is '-'. The '*' character will be treated specially as a wild-card character – see below for details.

*Input file format:*
`<Starting contents of tape>\n`
`<Starting offset of machine head>\n`
`<Start state index (integer)>\n`
`<Halting state index (integer)>\n`
`<Action table>\n`

The tape is initialized with the starting contents (which do not include the newline), and the head of the Turing machine is positioned at an offset (given by <Starting offset of machine head>) from the position of the first symbol of the initialized section of the tape (so offset zero would be on the first symbol of the starting contents). If the initial tape contents are empty, then the offset is irrelevant (but must still be included as per the input file specification). Note that, during the operation of the machine, the head may move outside positions that were initialized with the starting contents of the tape. 

<Action table> is a list of Actions of the following form, and each Action is separated by a newline:

<State index> <Read> <Write> <Direction> <New state index>

<State index> and <New state index> are integer state numbers. <Read> is either a symbol to match against, or the special character '*' to match against any symbol, to determine which transition to select when in state <State index>. <Write> is either the symbol to write back to the tape or '*' to indicate that no symbol should be written. <New state index> is the state number to transition to. <Direction> is either 1, 0, or -1 to indicate that the head should be moved right, stay still, or moved left, respectively.Use of the wild-card character may mean that the processing order of rules is important. For the purpose of this exercise, you can assume that a rule containing a wild-card match will appear last in the list of rules for that state, and that a more precise match should always be processed first.

#include "lispi.h"
#include "parser.h"
#include "eval.h"
#include "primitive.h"

int main(int argc, char **argv)
{
	VALUE p, ret;
	environment *env;

	env = init_top_env();
	init_pbuff();
	
    init_cl();
	while(1) {
		get_cl();
		p = parse();
		Dlist(p);

		ret = eval(p, env);
		if(!IS_UNDEF(ret))
			Plist(ret);
	}
	
	return 0;
}

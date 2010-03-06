
#include "lispi.h"
#include "parser.h"
#include "eval.h"
#include "primitive.h"

int main(int argc, char **argv)
{
	VALUE p, ret;
	environment *env;
	FILE *fp;

	env = init_top_env();
	init_pbuff();
	
	if(argc >= 2) {
		D();
		fp = fopen(argv[1],"r");

		while(-1 != read_file(fp)) {
			p = parse();
			Dlist(p);
			ret = eval(p, env);
			if(!IS_UNDEF(ret))
				Plist(ret);
		}
		fclose(fp);

		return 0;
	}

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

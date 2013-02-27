#include <mcl_log.h>

int main(int argc, char *argv[])
{
	if (argc != 2) return;

	mcl_log *logger = mcl_log_new(argv[1]);

	MCLOG_INFO(logger, "test test test !!!");

	char *str = "you";
	int i = 20;

	MCLOG_WARN(logger, "what the fuck of %s %d", str, i);

	while (i >= 0) {
		MCLOG_DEBUG(logger, "%d", i --);
	}

	mcl_log_destroy(logger);

	return 1;
}


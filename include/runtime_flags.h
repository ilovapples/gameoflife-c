#ifndef RUNTIME_FLAGS_H
#define RUNTIME_FLAGS_H

#define BIT(i) (1<<(i))
#define FLAG_SET(f) ((runtime_flags & f) != 0)
#define SET_FLAG(f) (runtime_flags |= f)
#define IFDBG if (FLAG_SET(DEBUG))

enum FLAGS {
	DEBUG = BIT(0),
	QUIET = BIT(1),
	FORCE = BIT(2)
};

#endif /* RUNTIME_FLAGS_H */

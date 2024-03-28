PROG = ttt
CFLAGS := -g -Wall -Wextra -std=c11
CFLAGS += -I. -MMD
LDFLAGS :=
TRAIN = train
RL = rl
MCTS = mcts
RL_CFLAGS := $(CFLAGS) -D USE_RL
MCTS_CFLAGS := $(CFLAGS) -D USE_MCTS
MCTS_LDFLAGS := $(LDFLAGS) -lm

all: $(PROG)

OBJS := \
	game.o \
	mt19937-64.o \
	zobrist.o \
	agents/negamax.o \
	start.o
deps := $(OBJS:%.o=%.d)
deps += $(RL).d
deps += $(TRAIN).d
deps += $(MCTS).d

ttt_mode: $(MODE) $(OBJS)

$(OBJS) : %.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(RL): start.c agents/reinforcement_learning.c game.c
	$(CC) -o start.o -c start.c $(RL_CFLAGS)
	$(CC) -o agents/reinforcement_learning.o -c agents/reinforcement_learning.c $(RL_CFLAGS)
	$(CC) -o game.o -c game.c $(RL_CFLAGS)
	

$(TRAIN): $(TRAIN).c agents/reinforcement_learning.c game.c
	$(CC) -o train.o -c train.c $(CFLAGS)
	$(CC) -o agents/reinforcement_learning.o -c agents/reinforcement_learning.c $(CFLAGS)
	$(CC) -o game.o -c game.c $(CFLAGS)
	


$(MCTS): start.c agents/mcts.c game.c
	$(CC) -o start.o -c start.c $(MCTS_CFLAGS) $(MCTS_LDFLAGS)
	$(CC) -o agents/mcts.o -c agents/mcts.c $(MCTS_CFLAGS) $(MCTS_LDFLAGS)
	$(CC) -o game.o -c game.c $(MCTS_CFLAGS) $(MCTS_LDFLAGS)


# for i in $^ ; do \
# 	$(CC) -o $@ -c $$i $(MCTS_CFLAGS) $(MCTS_LDFLAGS); \
# done
clean:
	-$(RM) $(PROG) $(OBJS) $(deps) $(TRAIN) $(RL) $(MCTS)
	-$(RM) *.bin

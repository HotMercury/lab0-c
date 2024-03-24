PROG = ttt
CFLAGS := -Wall -Wextra -std=c11
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

lab0: $(OBJS)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(RL): main.c agents/reinforcement_learning.c game.c
	$(CC) -o $@ $^ $(RL_CFLAGS)

$(TRAIN): $(TRAIN).c agents/reinforcement_learning.c game.c
	$(CC) $(CFLAGS) -o $@ $^

$(MCTS): main.c agents/mcts.c game.c
	$(CC) -o $@ $^ $(MCTS_CFLAGS) $(MCTS_LDFLAGS)

clean:
	-$(RM) $(PROG) $(OBJS) $(deps) $(TRAIN) $(RL) $(MCTS)
	-$(RM) *.bin

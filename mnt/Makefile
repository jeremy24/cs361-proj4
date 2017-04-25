#scheduler Makefile
#Stephen Marz
#30 June 2016

PROG = Graph


CXX	 := g++
MV       ?= mv
RM       ?= rm
ECHO	 ?= echo

LDFLAGS	 :=
LIBS	 := 


# -O3  -msse2 -mavx

# CXXFLAGS := -MD -MP
CXXFLAGS := -fopenmp -mavx -mavx2 -Wall  -Wno-unused-function
CXXFLAGS += -std=c++11





default:
	@$(ECHO) This makefile uses g++-5. 
	@$(ECHO) Please install it or change to an older version if it does not work.
	@$(CXX) $(CXXFLAGS) Graph.cpp minimalDS.cpp -o $(PROG)
	@$(CXX) $(CXXFLAGS) -c Graph.* 
	@$(CXX) $(CXXFLAGS) -c minimalDS.cpp 
	@$(CXX) $(CXXFLAGS) -o $(PROG) *.o
	
	# @rm Graph.d
	
clean:
	@$(RM) Graph 
	@$(RM) *.o *.gch *.s *.ii
	


# default:
# 	@if [ ! -d $(DEP_PATH) ]; then mkdir $(DEP_PATH); fi
# 	@if [ ! -d $(OBJ_PATH) ]; then mkdir $(OBJ_PATH); fi
# 	@$(MAKE) $(PROG)

# $(PROG): $(OBJS) Makefile
# 	@echo "  LINK $@"
# 	@$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

# include $(DEPS)

# $(OBJ_PATH)/%.o: %.cpp Makefile
# 	@echo "   CXX $< -> $@"
# 	@$(CXX) $(CXXFLAGS) -o $@ -c $< 
# 	@$(MV) $(OBJ_PATH)/$(patsubst %.cpp,%.d,$<) $(DEP_PATH)/$(patsubst %.cpp,%.d,$<)

# $(OBJ_PATH)/%.o: %.c Makefile
# 	@echo "   CC  $< -> $@"
# 	@$(CC) $(CCFLAGS) -o $@ -c $<
# 	@$(MV) $(OBJ_PATH)/$(patsubst %.c,%.d,$<) $(DEP_PATH)/$(patsubst %.c,%.d,$<)

# .PHONY: clean distclean perm run

# oclean:
# 	@echo "  CLEAN"
# 	@$(RM) -f $(DEPS)
# 	@$(RM) -f $(OBJS)
# 	@$(RM) -f $(PROG)

# clean:
# 	@echo "  DISTCLEAN"
# 	@$(RM) -fr $(DEP_PATH)
# 	@$(RM) -fr $(OBJ_PATH)
# 	@$(RM) -f $(PROG)

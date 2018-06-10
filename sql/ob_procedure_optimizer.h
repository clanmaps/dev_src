/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_optimizer.h
 * @brief procedure optimizer class definition,used to adjust instactions execution sequence
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_27
 */

#ifndef OBPROCEDUREOPTIMIZER_H
#define OBPROCEDUREOPTIMIZER_H

//#define NOT_OPTIMIZE_FOR_COMPOSITE_STRUCTURE

#include "ob_procedure.h"

namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObProcDepGraph class
     * ObProcDepGraph is design for store procedure instructions depend relation
     */
    class ObProcDepGraph
    {
      public:
        const static int MAX_GRAPH_SIZE = 128;   ///< max instruction number in graph
        enum GraphType
        {
          Forward,
          Backward
        };
        /**
         * @brief The SpNode struct
         * node represent a instruction
         */
        struct SpNode
        {
            SpNode() : id_(0), next_(NULL) {}
            SpNode(int64_t id, SpNode *nxt) : id_(id), next_(nxt) {}
            int64_t id_;
            int     dep_type_;
            SpNode *next_;
        };
        /**
         * @brief set_insts
         * init inst_list_ and graph_
         * @param insts instruction array
         * @return error code
         */
        int set_insts(ObIArray<SpInst*> &insts);
        /**
         * @brief reorder_for_group
         * adjust instruction execute sequence base on graph_
         * @param seq adjusted execute sequence
         * @return error code
         */
        int reorder_for_group(ObIArray<int64_t> &seq);
        /**
         * @brief split
         * split nested block
         * @param seq  adjusted execute sequence
         * @param pre_count access basedata instructions number
         * @return error code
         */
        int split(ObIArray<int64_t> &seq, int64_t &pre_count);
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return byte number
         */
        int64_t to_string(char *buf, const int64_t buf_len) const;

      private:
        /**
         * @brief build_dep_graph
         * build depend relation
         * @param type
         */
        void build_dep_graph(GraphType type = Forward);
        /**
         * @brief mark_done
         * mark node and depend node degree minus one
         * @param id node location in graph_
         * @param type graph type
         */
        void mark_done(int64_t id, GraphType type = Forward);
        /**
         * @brief is_leaf
         * judge node whether the leaf node
         * @param id node location in graph_
         * @return bool value
         */
        bool is_leaf(int64_t id) const;
        /**
         * @brief is_root
         * judge node whether the root node
         * @param id node location in graph_
         * @return bool value
         */
        bool is_root(int64_t id) const;
        /**
         * @brief is_done
         * judge node whether marked
         * @param id node location in graph_
         * @return bool value
         */
        bool is_done(int64_t id) const;
        /**
         * @brief is_stype
         * judge node instruction type whether ms rpc
         * @param id node location in graph_
         * @return bool value
         */
        bool is_stype(int64_t id) const;
        /**
         * @brief is_ttype
         * judge node instruction type whether ups rpc
         * @param id node location in graph_
         * @return bool value
         */
        bool is_ttype(int64_t id) const;
        /**
         * @brief cover_tnode
         * judge node instruction whether cpver by ups rpc node
         * @param id node location in graph_
         * @return bool value
         */
        bool cover_tnode(int64_t id) const;
        /**
         * @brief could_split
         * judge whether split nested block
         * @return bool value
         */
        bool could_split() const;
        /**
         * @brief tag_true_flow
         * tag true depend node
         * @param id node location in graph_
         * @return error code
         */
        int tag_true_flow(int64_t id);
        /**
         * @brief check_dep
         * check dependence between instructions
         * @param in_node first instruction node
         * @param out_node second instruction node
         * @return depend type
         */
        static int check_dep(SpInst *in_node, SpInst *out_node);
        /**
         * @brief add_edge
         * add depend ralation between instruction nodes
         * @param i first instruction node
         * @param j second instruction node
         * @param dep_type depend type
         */
        void add_edge(int64_t i, int64_t j, int dep_type);

        ObSEArray<SpInst *, MAX_GRAPH_SIZE> inst_list_;  ///<  instruction list
        ObSEArray<SpNode, MAX_GRAPH_SIZE> graph_;  ///<  depend graph
        ObSEArray<int, MAX_GRAPH_SIZE> degree_;   ///<  nodes degree
        ObSEArray<bool, MAX_GRAPH_SIZE> cover_trpc_;  ///<  nodes cover ralation
        ObSEArray<bool, MAX_GRAPH_SIZE> flow_srpc_;   ///< true data depend
        int64_t active_node_count_;   ///<  undone node
        GraphType type_;  ///< graph type
        ModuleArena arena_;   ///< memory allocator
        bool inited_;   ///< init flag
    };

    //Optimizer should handle the following structures:
    //sequential, loop, control instructions.
    /**
     * @brief The ObProcedureOptimizer class
     * procdure optimizer include some static function
     * used to optimize instruction execution
     */
    class ObProcedureOptimizer
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureOptimizer();
        /**
         * @brief optimize
         * optimize procedure instruction execution
         * @param proc procedure phyoperator object
         * @param no_group flag of no group execution
         * @return error code
         */
        static int optimize(ObProcedure &proc, bool no_group);

      private:
        /**
         * @brief rule_based_optimize
         * Basic optimize method
         * @param proc procedure phyoperator object
         * @return
         */
        static int rule_based_optimize(ObProcedure &proc);

        /**
         * @brief no_optimize
         * no optimize instruction execution
         * @param proc procedure phyoperator object
         * @return error code
         */
        static int no_optimize(ObProcedure &proc);

      private:


#ifndef NOT_OPTIMIZE_FOR_COMPOSITE_STRUCTURE

        /**
         * @brief loop_split
         * Optimize is possible when there is no inter-loop dependence.
         * Checking the invariant of the loop footscript.
         * @param inst loop instruction
         * @param inst_list instruction list
         * @return error code
         */
        static int loop_split(SpLoopInst *loop_inst, SpInstList &inst_list);
        /**
         * @brief ctrl_split
         * split if control block instructions
         * @param if_inst if control instructions
         * @param inst_list instruction list
         * @return error code
         */
        static int ctrl_split(SpIfCtrlInsts *if_inst, SpInstList &inst_list);

#endif

        /**
         * @brief split
         * Given a list of instructions, we can preexecute S-RPC in a separate context
         * If S-RPC is dominated by only LPC, they can be pre-executed.
         * If S-RPC is dominated by any T-RPC, they cannot be pre-executed.
         * @param inst_list instruction list
         * @param pre_insts pre-executed instruction list
         * @param post_insts post-executed instruction list
         * @return error code
         */
        static int do_split(SpInstList &inst_list, SpInstList &pre_insts, SpInstList &post_insts);

        /**
         * @brief do_loop_split
         * expend twice as much of loop instructions then judge whether can be pre-executed
         * @param inst_list instruction list
         * @param pre_insts  pre-executed instruction list
         * @param post_insts post-executed instruction list
         * @return error code
         */
        static int do_loop_split(SpInstList &inst_list, SpInstList &pre_insts, SpInstList &post_insts);

        /**
         * @brief group
         * merge constantly ups-rpc instruction to group to execute
         * @param proc procedure phyoperator object save instruction execute list
         * @param expand_list raw instruction list
         * @return error code
         */
        static int group(ObProcedure &proc, SpInstList &expand_list);

      private:

    };
  }
}

#endif // OBPROCEDUREOPTIMIZER_H

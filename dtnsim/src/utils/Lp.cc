#include "Lp.h"

#ifdef USE_CPLEX_LIBRARY
#ifdef USE_BOOST_LIBRARIES

Lp::Lp(ContactPlan *contactPlan, int nodesNumber, map<int, map<int, map<int, double>>> traffic) {
    contactPlan_ = contactPlan;
    nodesNumber_ = nodesNumber;
    traffic_ = traffic;

    this->populateCommodities();

    // fill states with RouterGraphs with empty flows

    // compute topology
    map<double, TopologyGraph> topology = topologyUtils::computeTopology(contactPlan, nodesNumber);
    // create empty router graphs as a copy of topology
    map<double, TopologyGraph>::iterator it1 = topology.begin();
    map<double, TopologyGraph>::iterator it2 = topology.end();
    for (; it1 != it2; ++it1) {
        // create routeGraph and add vertices
        RouterGraph routerGraph;
        for (int i = 1; i <= nodesNumber; i++) {
            RouterGraph::vertex_descriptor vertex = add_vertex(routerGraph);
            routerGraph[vertex].eid = i;

            // todo set the real buffer capacities (sdr size)
            routerGraph[vertex].bufferCapacity = numeric_limits<double>::max();
        }

        // double stateTime = it1->first;
        TopologyGraph topologyGraph = (it1->second);

        routerGraph[graph_bundle].stateStart = topologyGraph[graph_bundle].stateStart;
        routerGraph[graph_bundle].stateEnd = topologyGraph[graph_bundle].stateEnd;

        // fill routerGraph starting from topologyGraph
        TopologyGraph::edge_iterator ei, ei_end;
        for (tie(ei, ei_end) = edges(topologyGraph); ei != ei_end; ++ei) {
            int edgeId = topologyGraph[*ei].id;
            int v1Id = topologyGraph[source(*ei, topologyGraph)].eid;
            int v2Id = topologyGraph[target(*ei, topologyGraph)].eid;

            // find vertices corresponding to v1Id and v2Id in RouterGraph and save them
            // in vertexSource and vertexDestination
            int foundVertices = 0;
            RouterGraph::vertex_iterator vi1, vi2;
            tie(vi1, vi2) = vertices(routerGraph);
            RouterGraph::vertex_descriptor vertexSource;
            RouterGraph::vertex_descriptor vertexDestination;
            for (; vi1 != vi2; ++vi1) {
                RouterGraph::vertex_descriptor vertex = *vi1;
                if (routerGraph[vertex].eid == v1Id) {
                    vertexSource = vertex;
                    ++foundVertices;
                } else if (routerGraph[vertex].eid == v2Id) {
                    vertexDestination = vertex;
                    ++foundVertices;
                }
                if (foundVertices == 2) {
                    break;
                }
            }

            // add edge to found vertices in RouterGraph
            RouterGraph::edge_descriptor edge =
                (add_edge(vertexSource, vertexDestination, routerGraph)).first;
            routerGraph[edge].id = edgeId;
            routerGraph[edge].stateCapacity = topologyGraph[*ei].stateCapacity;
        }

        states_.push_back(routerGraph);
    }

    // fill intervals with the state intervals
    for (size_t i = 0; i < states_.size(); i++) {
        RouterGraph graph = states_.at(i);
        int point1 = graph[graph_bundle].stateStart;
        int point2 = graph[graph_bundle].stateEnd;
        intervals_.push_back(make_pair(point1, point2));
    }

    assert(states_.size() == intervals_.size());

    model_ = IloModel(env_);
    dVars_ = IloNumVarArray(env_);
    constraints_ = IloRangeArray(env_);

    populateModel();
    populateCplex();
}

Lp::~Lp() {
    env_.end();
}

void Lp::populateCommodities() {
    map<int, map<int, map<int, double>>>::iterator it1 = traffic_.begin();
    map<int, map<int, map<int, double>>>::iterator it2 = traffic_.end();

    for (; it1 != it2; ++it1) {
        // int state = it1->first;
        map<int, map<int, double>> m1 = it1->second;

        map<int, map<int, double>>::iterator it3 = m1.begin();
        map<int, map<int, double>>::iterator it4 = m1.end();

        for (; it3 != it4; ++it3) {
            int src = it3->first;
            map<int, double> m2 = it3->second;
            map<int, double>::iterator it5 = m2.begin();
            map<int, double>::iterator it6 = m2.end();
            for (; it5 != it6; ++it5) {
                double dst = it5->first;
                commodities_.insert(make_pair(src, dst));
            }
        }
    }
}

void Lp::populateModel() {
    // References:

    // Kevin Fall
    // Routing in a Delay Tolerant Network

    // Juan Alonso and Kevin Fall
    // A Linear Programming Formulation of Flows over Time with Piecewise Constant Capacity and
    // Transit Times Intel Research, Berkley, IRB-TR-03-007

    // Juan A. Fraire, Pablo G. Madoery, Jorge M. Finochietto,
    // Traffic-aware contact plan design for disruption-tolerant space sensor networks,
    // Ad Hoc Networks, Volume 47, 1 September 2016, Pages 41-52, ISSN 1570-8705,
    // http://doi.org/10.1016/j.adhoc.2016.04.007.

    // bimapa de boost utilizado para acceder a los nombres e índices de las variables
    typedef bimap<string, int>::value_type bimapElement;

    // Se agregan variables y función objetivo al modelo (1)
    IloExpr fObj(env_);

    double tqAccum = 0;

    for (size_t s = 0; s < states_.size(); s++) {
        RouterGraph graph = states_.at(s);
        int tq = intervals_.at(s).second;

        tqAccum += tqAccum * (nodesNumber_) + 10;

        assert(tqAccum > 0);

        // Se agregan variables de transmisión
        RouterGraph::edge_iterator ei, ei_end;
        for (tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei) {
            set<pair<int, int>>::iterator it1 = commodities_.begin();
            for (; it1 != commodities_.end(); ++it1) {
                int k1 = it1->first;
                int k2 = it1->second;

                RouterGraph::vertex_descriptor u = source(*ei, graph);
                RouterGraph::vertex_descriptor v = target(*ei, graph);

                // x_s.u.v.k1.k2 = cantidad del commodity (k1,k2) que es transmitido en el arco u->v
                // en el estado s
                string xName = "x_" + lexical_cast<string>(s) + "." +
                               lexical_cast<string>(graph[u].eid) + "." +
                               lexical_cast<string>(graph[v].eid) + "." + lexical_cast<string>(k1) +
                               "." + lexical_cast<string>(k2);

                dVars_.add(IloNumVar(env_, 0, IloInfinity, IloNumVar::Int, xName.c_str()));
                variables_.insert(bimapElement(xName, dVars_.getSize() - 1));

                fObj.operator+=(tqAccum * dVars_[dVars_.getSize() - 1]);
            }
        }
        // Se agregan variables de ocupación de buffers
        RouterGraph::vertex_iterator vi, vi_end;
        for (tie(vi, vi_end) = vertices(graph); vi != vi_end; ++vi) {
            set<pair<int, int>>::iterator it1 = commodities_.begin();
            for (; it1 != commodities_.end(); ++it1) {
                int k1 = it1->first;
                int k2 = it1->second;

                int t = s;
                // n_t.v.k1.k2 = cantidad del commodity (k1,k2) que ocupa el buffer del nodo v en el
                // tiempo t
                string nName = "n_" + lexical_cast<string>(t) + "." +
                               lexical_cast<string>(graph[*vi].eid) + "." +
                               lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                dVars_.add(IloNumVar(env_, 0, IloInfinity, IloNumVar::Int, nName.c_str()));
                variables_.insert(bimapElement(nName, dVars_.getSize() - 1));

                if (graph[*vi].eid == k2) {
                    fObj.operator-=(pow(tq, 3) * dVars_[dVars_.getSize() - 1]);
                }
            }
        }

        // variables de buffers en el último t
        // que no estan contempladas porque se recorre por estados
        if (s == (states_.size() - 1)) {
            RouterGraph::vertex_iterator vi, vi_end;
            for (tie(vi, vi_end) = vertices(graph); vi != vi_end; ++vi) {
                // Se agrega variable final de ocupación de buffers
                // intervals_ = { [t0,t1], [t1,t2], [t2,t3], ... }
                // states = { s0, s1, s2, ... }
                // Se necesitan variables de ocupación de buffers nt0, nt1, nt2, nt3, ...
                set<pair<int, int>>::iterator it1 = commodities_.begin();
                for (; it1 != commodities_.end(); ++it1) {
                    int k1 = it1->first;
                    int k2 = it1->second;

                    // n_t.v.k1.k2 = cantidad del commodity (k1,k2) que ocupa el buffer del nodo v
                    // en el tiempo t
                    string nName = "n_" + lexical_cast<string>(s + 1) + "." +
                                   lexical_cast<string>(graph[*vi].eid) + "." +
                                   lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                    dVars_.add(IloNumVar(env_, 0, IloInfinity, IloNumVar::Int, nName.c_str()));
                    variables_.insert(bimapElement(nName, dVars_.getSize() - 1));
                }
            }
        }
    }

    model_.add(IloMinimize(env_, fObj, "K"));
    fObj.end();

    // Se agregan restricciones de conservación de flujo
    // teniendo en cuenta los buffers (2)
    for (size_t s = 0; s < states_.size(); s++) {
        RouterGraph graph = states_.at(s);

        RouterGraph::vertex_iterator vi, vi_end;
        for (tie(vi, vi_end) = vertices(graph); vi != vi_end; ++vi) {
            set<pair<int, int>>::iterator it1 = commodities_.begin();
            for (; it1 != commodities_.end(); ++it1) {
                int k1 = it1->first;
                int k2 = it1->second;

                IloExpr expr(env_);

                RouterGraph::in_edge_iterator iei, iei_end;
                for (tie(iei, iei_end) = in_edges(*vi, graph); iei != iei_end; ++iei) {
                    RouterGraph::vertex_descriptor u = source(*iei, graph);
                    RouterGraph::vertex_descriptor v = target(*iei, graph);

                    string rVarName = "x_" + lexical_cast<string>(s) + "." +
                                      lexical_cast<string>(graph[u].eid) + "." +
                                      lexical_cast<string>(graph[v].eid) + "." +
                                      lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                    int xVarIndex = variables_.left.at(rVarName);

                    expr.operator+=(dVars_[xVarIndex]);
                }

                RouterGraph::out_edge_iterator oei, oei_end;
                for (tie(oei, oei_end) = out_edges(*vi, graph); oei != oei_end; ++oei) {
                    RouterGraph::vertex_descriptor u = source(*oei, graph);
                    RouterGraph::vertex_descriptor v = target(*oei, graph);

                    string xVarName = "x_" + lexical_cast<string>(s) + "." +
                                      lexical_cast<string>(graph[u].eid) + "." +
                                      lexical_cast<string>(graph[v].eid) + "." +
                                      lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                    int xVarIndex = variables_.left.at(xVarName);
                    expr.operator-=(dVars_[xVarIndex]);
                }

                // n(tk+1)
                string t2VarName = "n_" + lexical_cast<string>(s + 1) + "." +
                                   lexical_cast<string>(graph[*vi].eid) + "." +
                                   lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                int t2VarIndex = variables_.left.at(t2VarName);
                expr.operator-=(dVars_[t2VarIndex]);

                // n(tk)
                string t1VarName = "n_" + lexical_cast<string>(s) + "." +
                                   lexical_cast<string>(graph[*vi].eid) + "." +
                                   lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                int t1VarIndex = variables_.left.at(t1VarName);
                expr.operator+=(dVars_[t1VarIndex]);

                if (graph[*vi].eid == k1) {
                    double traffic = this->getTraffic(s + 1, k1, k2);

                    if (traffic != 0) {
                        expr.operator+=(traffic);
                    }
                }

                IloRange c(expr == 0);
                c.setName((string("FlowConservationConstraint_n") +
                           lexical_cast<string>(graph[*vi].eid) + string("_t") +
                           lexical_cast<string>(s))
                              .c_str());

                constraints_.add(c);
                expr.end();
            }
        }
    }

    // Se agregan restricciones de ocupación de buffers (4)
    for (size_t s = 0; s < states_.size(); s++) {
        RouterGraph graph = states_.at(s);

        RouterGraph::vertex_iterator vi, vi_end;
        for (tie(vi, vi_end) = vertices(graph); vi != vi_end; ++vi) {
            IloExpr expr(env_);

            double bufferCapacity = graph[*vi].bufferCapacity;

            set<pair<int, int>>::iterator it1 = commodities_.begin();
            for (; it1 != commodities_.end(); ++it1) {
                int k1 = it1->first;
                int k2 = it1->second;

                string nVarName = "n_" + lexical_cast<string>(s) + "." +
                                  lexical_cast<string>(graph[*vi].eid) + "." +
                                  lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                int nVarIndex = variables_.left.at(nVarName);
                expr.operator+=(dVars_[nVarIndex]);
            }

            IloRange c(expr <= bufferCapacity);
            c.setName((string("BufferCapacityConstraint_n") + lexical_cast<string>(graph[*vi].eid))
                          .c_str());

            constraints_.add(c);
            expr.end();
        }

        // restricciones a los buffers en el último t
        // que no estan contempladas porque se recorre por estados
        if (s == states_.size() - 1) {
            RouterGraph::vertex_iterator viAux, viAux_end;
            for (tie(viAux, viAux_end) = vertices(graph); viAux != viAux_end; ++viAux) {
                IloExpr expr(env_);

                double bufferCapacity = graph[*viAux].bufferCapacity;

                set<pair<int, int>>::iterator it1 = commodities_.begin();
                for (; it1 != commodities_.end(); ++it1) {
                    int k1 = it1->first;
                    int k2 = it1->second;

                    string nVarName = "n_" + lexical_cast<string>(s + 1) + "." +
                                      lexical_cast<string>(graph[*viAux].eid) + "." +
                                      lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                    int nVarIndex = variables_.left.at(nVarName);
                    expr.operator+=(dVars_[nVarIndex]);
                }

                IloRange c(expr <= bufferCapacity);
                c.setName(
                    (string("BufferCapacityConstraint_n") + lexical_cast<string>(graph[*vi].eid))
                        .c_str());

                constraints_.add(c);
                expr.end();
            }
        }
    }

    // Se agregan restricciones de flujo en los arcos (5)
    for (size_t s = 0; s < states_.size(); s++) {
        RouterGraph graph = states_.at(s);

        RouterGraph::edge_iterator ei, ei_end;
        for (tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei) {
            RouterGraph::vertex_descriptor u = source(*ei, graph);
            RouterGraph::vertex_descriptor v = target(*ei, graph);

            double stateCapacity = graph[*ei].stateCapacity;

            IloExpr expr(env_);

            set<pair<int, int>>::iterator it1 = commodities_.begin();
            for (; it1 != commodities_.end(); ++it1) {
                int k1 = it1->first;
                int k2 = it1->second;

                string xVarName = "x_" + lexical_cast<string>(s) + "." +
                                  lexical_cast<string>(graph[u].eid) + "." +
                                  lexical_cast<string>(graph[v].eid) + "." +
                                  lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                int xVarIndex = variables_.left.at(xVarName);
                expr.operator+=(dVars_[xVarIndex]);
            }

            IloRange c(expr <= (stateCapacity));
            c.setName((string("ArcCapacityConstraint_e") + lexical_cast<string>(graph[u].eid) +
                       "." + lexical_cast<string>(graph[v].eid))
                          .c_str());

            constraints_.add(c);
            expr.end();
        }
    }

    // Se agregan restricciones de condiciones iniciales de los buffers (7)
    RouterGraph::vertex_iterator vi, vi_end;
    RouterGraph graph1 = states_.front();
    for (tie(vi, vi_end) = vertices(graph1); vi != vi_end; ++vi) {
        set<pair<int, int>>::iterator it1 = commodities_.begin();
        for (; it1 != commodities_.end(); ++it1) {
            int k1 = it1->first;
            int k2 = it1->second;

            IloExpr expr(env_);

            int t = 0;
            string nVarName = "n_" + lexical_cast<string>(t) + "." +
                              lexical_cast<string>(graph1[*vi].eid) + "." +
                              lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

            int nVarIndex = variables_.left.at(nVarName);

            expr.operator+=(dVars_[nVarIndex]);

            if (graph1[*vi].eid == k1) {
                IloRange c(expr == this->getTraffic(0, k1, k2));
                c.setName(
                    (string("InitialConditionConstraint_n") + lexical_cast<string>(graph1[*vi].eid))
                        .c_str());
                constraints_.add(c);
            } else {
                IloRange c(expr == 0);
                c.setName(
                    (string("InitialConditionConstraint_n") + lexical_cast<string>(graph1[*vi].eid))
                        .c_str());
                constraints_.add(c);
            }

            expr.end();
        }
    }

    // Se agregan restricciones de condiciones intermedias de los buffers
    for (size_t s = 1; s < states_.size(); s++) {
        RouterGraph graph3 = states_.at(s);
        for (tie(vi, vi_end) = vertices(graph3); vi != vi_end; ++vi) {
            set<pair<int, int>>::iterator it1 = commodities_.begin();
            for (; it1 != commodities_.end(); ++it1) {
                int k1 = it1->first;
                int k2 = it1->second;

                if (graph3[*vi].eid == k1) {
                    double traffic = this->getTraffic(s, k1, k2);

                    if (traffic != 0) {
                        IloExpr expr(env_);

                        string nVarName = "n_" + lexical_cast<string>(s) + "." +
                                          lexical_cast<string>(graph3[*vi].eid) + "." +
                                          lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                        int nVarIndex = variables_.left.at(nVarName);

                        expr.operator+=(dVars_[nVarIndex]);

                        IloRange c(expr >= traffic);
                        c.setName((string("InitialConditionConstraint_n") +
                                   lexical_cast<string>(graph3[*vi].eid))
                                      .c_str());
                        constraints_.add(c);

                        expr.end();
                    }
                }
            }
        }
    }

    // Se agregan restricciones de condiciones finales de los buffers (12)
    RouterGraph graph2 = states_.back();
    for (tie(vi, vi_end) = vertices(graph2); vi != vi_end; ++vi) {
        IloExpr expr(env_);
        set<pair<int, int>>::iterator it1 = commodities_.begin();
        for (; it1 != commodities_.end(); ++it1) {
            int k1 = it1->first;
            int k2 = it1->second;

            if (graph2[*vi].eid == k2) {
                string nVarName = "n_" + lexical_cast<string>(states_.size()) + "." +
                                  lexical_cast<string>(graph2[*vi].eid) + "." +
                                  lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);

                int nVarIndex = variables_.left.at(nVarName);

                expr.operator+=(dVars_[nVarIndex]);
            }
        }

        IloRange c(expr == this->getTrafficForDestination(graph2[*vi].eid));
        c.setName(
            (string("FinalConditionConstraint_n") + lexical_cast<string>(graph2[*vi].eid)).c_str());
        constraints_.add(c);
        expr.end();
    }

    // Se agregan restricciones para evitar circulación de flujos
    for (size_t s = 0; s < states_.size(); s++) {
        RouterGraph graph = states_.at(s);
        RouterGraph::edge_iterator ei, ei_end;
        for (tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei) {
            RouterGraph::vertex_descriptor u = source(*ei, graph);
            RouterGraph::vertex_descriptor v = target(*ei, graph);
            pair<RouterGraph::edge_descriptor, bool> edRet = edge(v, u, graph);
            if (edRet.second) {
                set<pair<int, int>>::iterator it1 = commodities_.begin();
                for (; it1 != commodities_.end(); ++it1) {
                    int k1 = it1->first;
                    int k2 = it1->second;

                    string xVarNameUV = "x_" + lexical_cast<string>(s) + "." +
                                        lexical_cast<string>(graph[u].eid) + "." +
                                        lexical_cast<string>(graph[v].eid) + "." +
                                        lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);
                    string xVarNameVU = "x_" + lexical_cast<string>(s) + "." +
                                        lexical_cast<string>(graph[v].eid) + "." +
                                        lexical_cast<string>(graph[u].eid) + "." +
                                        lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);
                    int xVarIndexUV = variables_.left.at(xVarNameUV);
                    int xVarIndexVU = variables_.left.at(xVarNameVU);
                    constraints_.add((dVars_[xVarIndexUV] != 0) + (dVars_[xVarIndexVU] != 0) <= 1);
                }
            }
        }
    }

    model_.add(constraints_);
}

void Lp::populateCplex() {
    cplex_ = IloCplex(model_);
    cplex_.setOut(env_.getNullStream());
}

void Lp::exportModel(string fileLocation) {
    cplex_.exportModel((fileLocation + string(".lp")).c_str());
}

bool Lp::solve() {
    return cplex_.solve();
}

void Lp::printSolution() {
    IloNumArray vals(env_);
    env_.out() << "Solution status = " << cplex_.getStatus() << endl;
    env_.out() << "Solution value  = " << cplex_.getObjValue() << endl;

    cplex_.getValues(vals, dVars_);

    for (int i = 0; i < vals.getSize(); i++) {
        cout << variables_.right.at(i) << " = " << vals[i] << endl;
    }
}

vector<RouterGraph> Lp::getSolvedStates(int nSolution) {
    int numsol = cplex_.getSolnPoolNsolns();
    if (nSolution >= numsol) {
        cout << "Error: nSolution = " + lexical_cast<string>(nSolution) + " debe ser menor a " +
                    lexical_cast<string>(numsol)
             << endl;
    }
    IloNumArray vals(env_);
    cplex_.getValues(vals, dVars_, nSolution);
    return this->getSolvedStates(vals);
}

vector<RouterGraph> Lp::getSolvedStates(IloNumArray vals) {
    vector<RouterGraph> solvedStates(states_.size());
    for (size_t s = 0; s < states_.size(); s++) {
        RouterGraph graph = states_.at(s);

        set<pair<int, int>>::iterator it1 = commodities_.begin();
        for (; it1 != commodities_.end(); ++it1) {
            int k1 = it1->first;
            int k2 = it1->second;

            RouterGraph::edge_iterator ei, ei_end;
            for (tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei) {
                RouterGraph::vertex_descriptor u = source(*ei, graph);
                RouterGraph::vertex_descriptor v = target(*ei, graph);
                // x_s.u.v.k1.k2 = cantidad del commodity (k1,k2) que es transmitido en el arco u->v
                // en el estado s
                string xName = "x_" + lexical_cast<string>(s) + "." +
                               lexical_cast<string>(graph[u].eid) + "." +
                               lexical_cast<string>(graph[v].eid) + "." + lexical_cast<string>(k1) +
                               "." + lexical_cast<string>(k2);
                double xValue = vals[variables_.left.at(xName)];
                graph[*ei].flows[k1][k2] = xValue;
            }
        }

        RouterGraph::vertex_iterator vi, vi_end;
        for (tie(vi, vi_end) = vertices(graph); vi != vi_end; ++vi) {
            set<pair<int, int>>::iterator it1 = commodities_.begin();
            for (; it1 != commodities_.end(); ++it1) {
                int k1 = it1->first;
                int k2 = it1->second;

                // n_t.v.k1.k2 = cantidad del commodity (k1,k2) que ocupa el buffer del nodo v en el
                // tiempo t inicial
                string nInitialName = "n_" + lexical_cast<string>(s) + "." +
                                      lexical_cast<string>(graph[*vi].eid) + "." +
                                      lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);
                double nInitialValue = vals[variables_.left.at(nInitialName)];
                graph[*vi].initialBufferOccupancy[k1][k2] = nInitialValue;

                // n_t.v.k1.k2 = cantidad del commodity (k1,k2) que ocupa el buffer del nodo v en el
                // tiempo t inicial
                string nFinalName = "n_" + lexical_cast<string>(s + 1) + "." +
                                    lexical_cast<string>(graph[*vi].eid) + "." +
                                    lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);
                double nFinalValue = vals[variables_.left.at(nFinalName)];
                graph[*vi].finalBufferOccupancy[k1][k2] = nFinalValue;
            }
        }

        solvedStates.at(s) = graph;
    }

    return solvedStates;
}

double Lp::getObjectiveValue(int nSolution) {
    return cplex_.getObjValue(nSolution);
}

IloNumArray Lp::getSolution(int nSolution) {
    IloNumArray vals(env_);
    cplex_.getValues(vals, dVars_, nSolution);
    return vals;
}

bool Lp::computeNSolutions(int n, double solnPoolGap) {
    // SolPool -> agrupación de soluciones en cplex

    // Porcentage que una solución subóptima se puede alejar del óptimo en SolPool (ej 0%, 10%,
    // 100%)
    cplex_.setParam(IloCplex::SolnPoolGap, solnPoolGap);

    // Esfuerzo (tiempo y memoria) invertido para generar mayor cantidad de soluciones (1-4)
    cplex_.setParam(IloCplex::SolnPoolIntensity, 4);

    // Número máximo de soluciones conservadas en SolPool
    cplex_.setParam(IloCplex::SolnPoolCapacity, n);

    // Número máximo de soluciones generadas para SolPool
    // si se estan buscando soluciones subóptimas
    // tiene sentido que este valor sea mayor a SolnPoolCapacity
    // para que queden en SolPool mejores soluciones
    cplex_.setParam(IloCplex::PopulateLim, 10 * n);

    // Designa la estrategia para sustituir una solución en la agrupación de soluciones cuando esta
    // ha alcanzado su capacidad El valor 1 mantiene las soluciones con los mejores valores
    // objetivos El valor 2 sustituye soluciones para poder crear un conjunto de diversas soluciones
    cplex_.setParam(IloCplex::SolnPoolReplace, 1);

    // limita el tiempo de ejecución en segundos
    // cplex_.setParam(IloCplex::TiLim, 10);

    return cplex_.populate();
}

void Lp::printSolutions() {
    int numsol = cplex_.getSolnPoolNsolns();
    cout << "numSolutions = " << numsol << endl;
    for (int i = 0; i < numsol; i++) {
        IloNumArray vals(env_);
        cplex_.getValues(vals, dVars_, i);
        cout << "\nsolution " << i << ", obj = " << cplex_.getObjValue(i) << endl;
    }
}

map<int, map<pair<int, int>, double>> Lp::getRoutedTraffic(int nodeNumber, int solutionNumber) {
    // traffic_[contactId][pair<k1, k2>] = traffic

    map<int, map<pair<int, int>, double>> winTraffic;
    IloNumArray vals(env_);
    cplex_.getValues(vals, dVars_, solutionNumber);
    for (size_t s = 0; s < states_.size(); s++) {
        RouterGraph graph = states_.at(s);
        RouterGraph::edge_iterator ei, ei_end;
        for (tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei) {
            RouterGraph::vertex_descriptor u = source(*ei, graph);
            RouterGraph::vertex_descriptor v = target(*ei, graph);
            int id = graph[*ei].id;
            Contact contact = *contactPlan_->getContactById(id);
            int source = contact.getSourceEid();
            int destination = contact.getDestinationEid();

            set<pair<int, int>>::iterator it1 = commodities_.begin();
            for (; it1 != commodities_.end(); ++it1) {
                int k1 = it1->first;
                int k2 = it1->second;

                if (k1 == nodeNumber) {
                    // x_s.u.v.k1.k2 = cantidad del commodity (k1,k2) que es transmitido en el arco
                    // u->v en el estado s
                    string xName = "x_" + lexical_cast<string>(s) + "." +
                                   lexical_cast<string>(graph[u].eid) + "." +
                                   lexical_cast<string>(graph[v].eid) + "." +
                                   lexical_cast<string>(k1) + "." + lexical_cast<string>(k2);
                    double xValue = vals[variables_.left.at(xName)];

                    if (xValue != 0) {
                        if ((source == graph[u].eid) && (destination == graph[v].eid)) {
                            pair<int, int> commodity(k1, k2);
                            double traffic = winTraffic[id][commodity];
                            traffic += xValue;
                            winTraffic[id][commodity] = traffic;
                        }
                    }
                }
            }
        }
    }

    return winTraffic;
}

/// retorna el tráfico generado en un estado
double Lp::getTraffic(int state, int k1, int k2) {
    map<int, map<int, map<int, double>>>::iterator it0 = traffic_.find(state);

    if (it0 != traffic_.end()) {
        map<int, map<int, double>> m1 = it0->second;
        map<int, map<int, double>>::iterator it1 = m1.find(k1);

        if (it1 != m1.end()) {
            map<int, double> m2 = it1->second;
            map<int, double>::iterator it2 = m2.find(k2);

            if (it2 != m2.end()) {
                return it2->second;
            }
        }
    }

    return 0;
}

double Lp::getTrafficForDestination(int k2) {
    double k2Traffic = 0;

    map<int, map<int, map<int, double>>>::iterator it0 = traffic_.begin();
    for (; it0 != traffic_.end(); ++it0) {
        map<int, map<int, double>> m1 = it0->second;
        map<int, map<int, double>>::iterator it1 = m1.begin();
        for (; it1 != m1.end(); ++it1) {
            map<int, double> m2 = it1->second;
            map<int, double>::iterator it2 = m2.find(k2);

            if (it2 != m2.end()) {
                k2Traffic += it2->second;
            }
        }
    }

    return k2Traffic;
}

map<int, double> Lp::getUsedContacts(int solutionNumber) {
    map<int, double> contactsTraffic;
    IloNumArray vals(env_);
    cplex_.getValues(vals, dVars_, solutionNumber);
    for (size_t s = 0; s < states_.size(); s++) {
        RouterGraph graph = states_.at(s);
        RouterGraph::edge_iterator ei, ei_end;
        for (tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei) {
            RouterGraph::vertex_descriptor u = source(*ei, graph);
            RouterGraph::vertex_descriptor v = target(*ei, graph);
            int contactId = graph[*ei].id;
            Contact contact = *contactPlan_->getContactById(contactId);
            int src = contact.getSourceEid();
            int dst = contact.getDestinationEid();
            double totalContactTraffic = 0;

            set<pair<int, int>>::iterator it1 = commodities_.begin();
            for (; it1 != commodities_.end(); ++it1) {
                int k1 = it1->first;
                int k2 = it1->second;

                // x_s.u.v.k1.k2 = cantidad del commodity (k1,k2) que es transmitido en el arco u->v
                // en el estado s
                string xName = "x_" + lexical_cast<string>(s) + "." +
                               lexical_cast<string>(graph[u].eid) + "." +
                               lexical_cast<string>(graph[v].eid) + "." + lexical_cast<string>(k1) +
                               "." + lexical_cast<string>(k2);
                double xValue = vals[variables_.left.at(xName)];
                totalContactTraffic += xValue;
            }

            if (totalContactTraffic != 0) {
                if ((src == graph[u].eid) && (dst == graph[v].eid)) {
                    contactsTraffic[contactId] = totalContactTraffic;
                }
            }
        }
    }

    return contactsTraffic;
}

vector<pair<int, int>> Lp::getIntervals() const {
    return intervals_;
}

void Lp::printTraffic() {
    cout << "printing traffics" << endl;

    map<int, map<int, map<int, double>>>::iterator it1 = traffic_.begin();
    map<int, map<int, map<int, double>>>::iterator it2 = traffic_.end();

    for (; it1 != it2; ++it1) {
        int state = it1->first;
        map<int, map<int, double>> m1 = it1->second;

        map<int, map<int, double>>::iterator it3 = m1.begin();
        map<int, map<int, double>>::iterator it4 = m1.end();

        for (; it3 != it4; ++it3) {
            int src = it3->first;
            map<int, double> m2 = it3->second;
            map<int, double>::iterator it5 = m2.begin();
            map<int, double>::iterator it6 = m2.end();
            for (; it5 != it6; ++it5) {
                int dst = it5->first;
                double size = it5->second;

                cout << "traffic: state=" << state << ", src=" << src << ", dst=" << dst
                     << ", size = " << size << endl;
            }
        }
    }
}

void Lp::printCommodities() {
    cout << "printing commodities" << endl;

    set<pair<int, int>>::iterator it1 = commodities_.begin();
    set<pair<int, int>>::iterator it2 = commodities_.end();

    for (; it1 != it2; ++it1) {
        pair<int, int> p = *it1;

        cout << p.first << " - " << p.second << endl;
    }
}

ContactPlan *Lp::getContactPlan() {
    return contactPlan_;
}

#endif /* USE_BOOST_LIBRARIES */
#endif /* USE_CPLEX_LIBRARY */

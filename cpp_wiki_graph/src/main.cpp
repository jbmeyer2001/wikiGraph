#include <pybind11/pybind11.h>
#include <set>
#include <string>
#include <unordered_map>
#include<iostream>

#include <typeinfo>

namespace py = pybind11;

class WikiGraph {
    private:
    std::unordered_map<std::string, std::string> redirects;
    std::unordered_map<std::string, std::set<std::string>> articles;
    bool creatingGraph = true;

    public:
    void addRedirect(std::string startLink, std::string redirectLink) {
        if (!creatingGraph) {
            std::cout << "**Cannot add a redirect after graph has been created**" << std::endl;
            return;
        }

        redirects.emplace(startLink, redirectLink);
    }

    void addArticle(std::string articleName, py::list hyperlinkedArticles) {
        if (!creatingGraph) {
            std::cout << "**Cannot add an article after graph has been created**" << std::endl;
            return;
        }

        std::set<std::string> articleLinks;

        for(auto it = hyperlinkedArticles.begin(); it != hyperlinkedArticles.end(); it++) {
            articleLinks.emplace(it->cast<std::string>());
        }

        articles.emplace(articleName, articleLinks);
    }

    void createGraph() {
        if (!creatingGraph) {
            std::cout << "**Cannot create graph twice**" << std::endl;
            return;
        }

        //remove the redirects that don't corrospond to an article
        std::set<std::string> to_remove;
        for(auto i = redirects.begin(); i != redirects.end(); i++) {
            if (articles.find(i->second) == articles.end()) {
                to_remove.emplace(i->first);
            }
        }

        for(auto i = to_remove.begin(); i != to_remove.end(); i++) {
            redirects.erase(*i);
        }

        //remove any hyperlinks that don't have an associated article or redirect, and replace any articles with their redirect
        for (auto i = articles.begin(); i != articles.end(); i++) {
            std::set<std::string> to_add;
            std::set<std::string> to_remove;
            std::set<std::string> *hyperlinks = &i->second;

            //iterate through all the hyperlinks in the current article
            for(auto j = hyperlinks->begin(); j != hyperlinks->end(); j++) {
                std::string article = *j;
                
                //if the hyperlink is a redirect, mark it for deletion and add the redirect
                if (redirects.find(article) != redirects.end()) {
                    to_remove.emplace(article);
                    to_add.emplace(redirects.find(article)->second);
                    continue;
                }

                //if the hyperlink isn't a redirect, and it isn't an article, then mark it for deletion
                if (articles.find(article) == articles.end()) {
                    to_remove.emplace(article);
                }
            }

            //to deal with issues of adding/removing elements of a set while iterating through it, add/remove elements afterwards
            for(auto j = to_add.begin(); j != to_add.end(); j++) {
                hyperlinks->emplace(*j);
            }

            for(auto j = to_remove.begin(); j != to_remove.end(); j++) {
                hyperlinks->erase(*j);
            }
        }

        //we no longer need the redirects graph, remove
        redirects.clear();

        creatingGraph = false;
    }

    void test() {
        std::cout << "graph: " << std::endl;
        
        for(auto i = articles.begin(); i != articles.end(); i++) {
            std::string title = i->first;
            std::set<std::string> hyperlinks = i->second;

            std::cout << title << " -> ";
            for(auto j = hyperlinks.begin(); j != hyperlinks.end(); j++) {
                std::cout << *j << ", ";
            }
            std::cout << std::endl;
        }
    }
};

PYBIND11_MODULE(_core, m) {
    py::class_<WikiGraph>(m, "WikiGraph")
        .def(py::init<>())
        .def("add_redirect", &WikiGraph::addRedirect)
        .def("add_article", &WikiGraph::addArticle)
        .def("create_graph", &WikiGraph::createGraph)
        .def("test", &WikiGraph::test);
}

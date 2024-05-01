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

    public:
    void addRedirect(std::string startLink, std::string redirectLink) {
        redirects.emplace(startLink, redirectLink);
    }

    void addArticle(std::string articleName, py::list hyperlinkedArticles) {
        std::set<std::string> articleLinks;

        for(auto it = hyperlinkedArticles.begin(); it != hyperlinkedArticles.end(); it++) {
            articleLinks.emplace(it->cast<std::string>());
        }

        articles.emplace(articleName, articleLinks);
    }

    void test() {
        for (auto it = articles.begin(); it != articles.end(); it++) {
            std::cout << it->first << ":" << std::endl;

            std::set<std::string> hyperlinks = it->second;
            for (auto it = hyperlinks.begin(); it != hyperlinks.end(); it++) {
                std::cout << "--" << *it << std::endl;
            }
        }
        for (auto it = redirects.begin(); it != redirects.end(); it++) {
            std::cout << it->first << " redirects to: " << it->second << std::endl;
        }
    }
};

PYBIND11_MODULE(_core, m) {
    py::class_<WikiGraph>(m, "WikiGraph")
        .def(py::init<>())
        .def("add_redirect", &WikiGraph::addRedirect)
        .def("add_article", &WikiGraph::addArticle)
        .def("test", &WikiGraph::test);
}

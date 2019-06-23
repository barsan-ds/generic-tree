#include <QtTest/QtTest>
#include <stdexcept>
#include <string>

#include <TreeDS/match>
#include <TreeDS/tree>

using namespace md;
using namespace std;

class PatternTest : public QObject {

    Q_OBJECT

    private slots:
    void construction();
    void simpleMatch();
    void test1();
};

void PatternTest::construction() {
    pattern p1(one());
    QCOMPARE(p1.mark_count(), 0);

    pattern p2(
        one(1)(
            one(2),
            one(3)));
    QCOMPARE(p2.mark_count(), 0);

    pattern p3(
        one('a')(
            cpt(one('b')),
            one('c')));
    QCOMPARE(p3.mark_count(), 1);

    pattern p4(
        cpt(
            one(string("alpha"))(
                cpt(one(string("beta"))),
                one(string("gamma")),
                cpt(one()))));
    QCOMPARE(p4.mark_count(), 3);

    pattern p5(
        cpt(cpt(
            star())));
    QCOMPARE(p5.mark_count(), 2);

    pattern p6(
        one()(
            cpt(star()(
                star()(
                    cpt(capture_name<'a', 'n', ' ', 'a'>(), one('a'))),
                cpt(one('b')(
                    cpt(star())))))));
    QCOMPARE(p6.mark_count(), 4);

    pattern p7(
        cpt(cpt(cpt(
            capture_name<'a'>(),
            star(string("string"))(
                cpt(capture_name<'t'>(), cpt(one())),
                cpt(one(string("b"))))))));
    QCOMPARE(p7.mark_count(), 6);
}

void PatternTest::simpleMatch() {
    binary_tree<int> tree1 {
        n(1)(
            n(2),
            n(3))};
    binary_tree<int> tree2 {
        n(1)(
            n(2),
            n(1)(
                n(2),
                n(3)))};
    binary_tree<int> result;
    nary_tree<int> nary_result;

    {
        pattern p(one(1));
        QVERIFY(p.match(tree1));
        p.assign_result(result);
        QCOMPARE(result, n(1));
        QVERIFY_EXCEPTION_THROWN(p.assign_result(nary_result), std::invalid_argument);

        QVERIFY(p.match(tree2));
        p.assign_result(result);
        QCOMPARE(result, n(1));
    }
    {
        pattern p(
            one()(
                one(2)));
        QVERIFY(p.match(tree1));
        p.assign_result(result);
        QCOMPARE(result, n(1)(n(2)));

        QVERIFY(p.match(tree2));
        p.assign_result(result);
        QCOMPARE(result, n(1)(n(2)));
    }
    {
        pattern p {
            one(1)(
                one(3))};
        QVERIFY(p.match(tree1));
        p.assign_result(result);
        QCOMPARE(result, n(1)(n(), n(3)));

        QVERIFY(!p.match(tree2));
        p.assign_result(result);
        QCOMPARE(result, n());
    }
    {
        pattern p {
            cpt(one(1)(
                cpt(capture_name<'b'>(), one(2))))};
        QVERIFY(p.match(tree1));
        p.assign_result(result);
        QCOMPARE(result, n(1)(n(2)));
        p.assign_mark(capture_index<2>(), result);
        QCOMPARE(result, n(2));
        p.assign_mark(capture_index<1>(), result);
        QCOMPARE(result, n(1)(n(2)));
        p.assign_mark(capture_name<'b'>(), result);
        QCOMPARE(result, n(2));
    }
    {
        pattern p {
            one()(
                one(),
                one(3))};
        QVERIFY(p.match(tree1));
        p.assign_result(result);
        QCOMPARE(result, n(1)(n(2), n(3)));

        QVERIFY(!p.match(tree2));
        p.assign_result(result);
        QCOMPARE(result, n());
    }
    {
        pattern p {
            one(1)(
                one(2),
                cpt(capture_name<'t'>(), star<quantifier::RELUCTANT>()))};
        QVERIFY(p.match(tree1));
        p.assign_result(result);
        QCOMPARE(result, n(1)(n(2)));

        QVERIFY(p.match(tree2));
        p.assign_result(result);
        QCOMPARE(result, n(1)(n(2)));
    }
    {
        pattern p {
            one(1)(
                one(2),
                cpt(
                    capture_name<'a'>(),
                    star<quantifier::RELUCTANT>()(
                        one(3))))};
        QVERIFY(p.match(tree1));
        p.assign_result(result);
        QCOMPARE(result, n(1)(n(2), n(3)));

        QVERIFY(p.match(tree2));
        p.assign_result(result);
        QCOMPARE(
            result,
            n(1)(
                n(2),
                n(1)(
                    n(),
                    n(3))));
    }
}

void PatternTest::test1() {
    binary_tree<char> tree {
        n('x')(
            n('a')(
                n('a')(
                    n(),
                    n('a')(
                        n('a')(
                            n('a'),
                            n('a')),
                        n('a')(
                            n('a')(
                                n(),
                                n('y')),
                            n('a')))),
                n('b')(
                    n('b'),
                    n('b')(
                        n('y')))),
            n('a'))};
    binary_tree<char> result;

    {
        pattern p(
            star()(
                one('a'),
                star('b')(
                    one('y'))));
        QVERIFY(p.match(tree));
        p.assign_result(result);
        QCOMPARE(
            result,
            n('x')(
                n('a')(
                    n('a'),
                    n('b')(
                        n('b'),
                        n('b')(
                            n('y')))),
                n('a')));
    }
    {
        pattern p(
            one()(
                star<quantifier::RELUCTANT>('a')(
                    one('y'),
                    one('b'))));
        QVERIFY(p.match(tree));
        p.assign_result(result);
        QCOMPARE(
            result,
            n('x')(
                n('a')(
                    n('a')(
                        n(),
                        n('a')(
                            n(),
                            n('a')(
                                n('a')(
                                    n(),
                                    n('y'))))),
                    n('b'))));
    }
    {
        pattern p(
            star()(
                cpt(capture_name<'P'>(),
                    star('a')(
                        one('a')(
                            one('a'),
                            one('a')))),
                cpt(capture_name<'b'>(),
                    star('b')(
                        cpt(capture_name<'y'>(),
                            star('y'))))));
        QVERIFY(p.match(tree));
        p.assign_result(result);
        QCOMPARE(
            result,
            n('x')(
                n('a')(
                    n('a')(
                        n(),
                        n('a')(
                            n('a'),
                            n('a'))),
                    n('b')(
                        n('b'),
                        n('b')(
                            n('y')))),
                n('a')));
        p.assign_mark(capture_name<'P'>(), result);
        QCOMPARE(
            result,
            n('a')(
                n(),
                n('a')(
                    n('a'),
                    n('a'))));
        p.assign_mark(capture_name<'y'>(), result);
        QCOMPARE(result, n('y'));
    }
    {
        pattern p {
            star()(
                star(),
                star(),
                star())};
        QVERIFY(p.match(tree));
        p.assign_result(result);
        QCOMPARE(result, n('x'));
    };
    {
        pattern p {
            star()(
                star(),
                one('x'),
                star())};
        QVERIFY(p.match(tree));
        p.assign_result(result);
        QCOMPARE(result, n('x'));
    };
}

QTEST_MAIN(PatternTest);
#include "PatternTest.moc"

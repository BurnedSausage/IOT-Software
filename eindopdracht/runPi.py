import numpy as np
from flask import Flask, jsonify, abort, make_response, request, url_for, g
from sqlite3 import dbapi2 as sqlite3

app = Flask(__name__)

@app.route('/statistics/', methods=['GET'])
def get_statistics():
        statistic = retrieve_statistic()

        return statistic

@app.route('/arduinos/<int:id>/measurements/', methods=['POST'])
def post_measurements(id):
        db = get_db()
        cur = db.execute("insert into arduinos (id, measurement) values (?, ?)", (id, request.json.get('measurement')))
        db.commit()

        return jsonify({"result":True})

@app.route('/measurements/', methods=['DELETE'])
def delete_measurements():
        db = get_db()
        cur = db.execute("delete from arduinos")
        db.commit()

        return jsonify({"result":True})

@app.errorhandler(400)
def bad_request(error):
        return make_response(jsonify({'error': 'Bad request'}), 400)

@app.errorhandler(404)
def not_found(error):
        return make_response(jsonify({'error': 'Not found'}), 404)

def get_db():
        if not hasattr(g, 'sqlite_db'):
                g.sqlite_db = connect_db()
        return g.sqlite_db

def connect_db():
        rv = sqlite3.connect('eindopdracht.db')
        rv.row_factory = sqlite3.Row
        return rv
def make_public_statistic(statistic):
        new_statistic = {}
        for field in statistic:
                if field == 'id':
                        new_statistic['uri'] = url_for('get_statistic', statistic_id=statistic['id'], _external=True)
                else:
                        new_statistic[field] = statistic[field]
        return new_statistic

def retrieve_statistic():
        x = get_x()
        y = get_y()
        s = np.polyfit(x, y, 2)
        p = np.poly1d(s*s)

        data = "function: %.2fx^2 , %.2fx + %.2f r^2 = %.2f" %(s[2], s[1], s[0], p[0])

        return data

def get_x():
        db = get_db()
        cur = db.execute("select measurement from arduinos where id = 1")
        rows = cur.fetchall()
        x = []
        for row in rows:
                x.append(row[0])

        return x

def get_y():
        db = get_db()
        cur = db.execute("select measurement from arduinos where id = 2")
        rows = cur.fetchall()
        y = []
        for row in rows:
                y.append(row[0])

        return y

@app.teardown_appcontext
def close_db(error):
        if hasattr(g, 'sqlite_db'):
                g.sqlite_db.close()

if __name__ == '__main__':
        app.run(debug=True)

import spacy

class NLPModel:
    def __init__(self):
        self.nlp = spacy.load("en_core_web_lg")

    def process_text(self, text):
        doc = self.nlp(text)
        results = []
        for token in doc:
            results.append((token.text, token.lemma_, token.pos_, token.dep_))
        return results

    def compare_similarity(self, question, answers):
        question_word = self.nlp(question)

        if not question_word.vector.any():
            question_word = self.nlp("tomato")

        similarities = {}
        for answer in answers:
            answer_word = self.nlp(answer)

            if not answer_word.vector.any():
                question_word = self.nlp("tomato")

            similarity = question_word.similarity(answer_word)
            similarities[answer] = similarity
        return similarities